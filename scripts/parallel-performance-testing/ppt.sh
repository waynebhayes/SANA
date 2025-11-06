#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SANA_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
OUTPUT_DIR="$SCRIPT_DIR/output"
RESULTS_DIR="$SCRIPT_DIR/results"
SBATCH_SCRIPTS_DIR="$SCRIPT_DIR/sbatch_scripts"
SANA_2_1="./sana2.1"
SANA_3_3="./sana3.3"

declare -a TEST_CONFIGS=(
    "2.1 1"
    "2.1 2"
    "2.1 4"
    "2.1 8"
    "2.1 16"
    "2.1 32" # Undergrads are capped at 20-core jobs on openlab
    "3.3 1"
    "3.3 2"
    "3.3 4"
    "3.3 8"
    "3.3 16"
    "3.3 32" # Undergrads are capped at 20-core jobs on openlab
)

declare -a DETECTION_ORDER=("TInitial" "ExecutionTime" "FinalScore")

declare -A DETECTIONS=(
    ["TInitial"]="Computed TInitial [^ ]+ in ([0-9.]+)s"
    ["ExecutionTime"]="Executed SANA[^ ]+ in ([0-9.]+)s"
    ["FinalScore"]="100%.*score = ([0-9.e+-]+)"
)

ensure_directories() {
    mkdir -p "$OUTPUT_DIR"
    mkdir -p "$RESULTS_DIR"
    mkdir -p "$SBATCH_SCRIPTS_DIR"
}

create_sbatch_script() {
    local sana_version="$1"
    local threads="$2"
    local output_file="$3"
    local script_path="$SBATCH_SCRIPTS_DIR/sbatch_${sana_version}_${threads}.sh"
    
    mkdir -p "$SBATCH_SCRIPTS_DIR"
    
    cat > "$script_path" <<-EOF
		#!/bin/bash
		#SBATCH --job-name=sana-PPT-${sana_version}-${threads}
		#SBATCH --nodes=1
		#SBATCH --ntasks-per-node=1
		#SBATCH --cpus-per-task=${threads}
		#SBATCH --nodelist=odin
		#SBATCH --output=${output_file}
		#SBATCH --error=${output_file}
		
		cd "$SANA_DIR"
		EOF

    if [ "$sana_version" = "2.1" ]; then
        echo "${SANA_2_1} -g1 yeast -g2 human -ec 1" >> "$script_path"
    else
        echo "${SANA_3_3} -g1 yeast -g2 human -ec 1 -maxthreads ${threads}" >> "$script_path"
    fi
    
    chmod 755 "$script_path"
    echo "$script_path"
}

clear_output() {
    ensure_directories
    
    local files_removed=0
    for dir in "$OUTPUT_DIR" "$SBATCH_SCRIPTS_DIR"; do
        [ -d "$dir" ] || continue
        for file in "$dir"/*; do
            [ -f "$file" ] || continue
            rm -f "$file"
            ((files_removed++)) || true
        done
    done
    
    echo "Cleared $files_removed file(s)"
}

submit_jobs() {
    ensure_directories
    
    echo "Creating sbatch scripts and submitting jobs..."
    echo ""
    
    declare -A job_ids
    declare -A job_status
    
    for config in "${TEST_CONFIGS[@]}"; do
        read -r version threads <<< "$config"
        local output_file="${OUTPUT_DIR}/${version}-${threads}.out"
        local key="${version}-${threads}"
        
        if [ -f "$output_file" ]; then
            job_status["$key"]="SKIPPED"
            continue
        fi
        
        local script
        script=$(create_sbatch_script "$version" "$threads" "$output_file")
        
        local result
        result=$(cd "$SBATCH_SCRIPTS_DIR" && sbatch "$(basename "$script")" 2>&1)
        
        if [ $? -ne 0 ] || ! [[ "$result" =~ Submitted\ batch\ job\ ([0-9]+) ]]; then
            echo "Error submitting ${version}-${threads}: $result" >&2
            job_status["$key"]="ERROR"
            continue
        fi
        
        job_ids["$key"]="${BASH_REMATCH[1]}"
    done
    
    echo "All jobs submitted. Monitoring..."
    echo ""
    
    while true; do
        local all_done=true
        local lines_to_clear=0
        
        printf "%-6s %-6s %-12s %-6s %-30s\n" "Version" "Cores" "JobID" "Time" "Slurm Status"
        printf "%-6s %-6s %-12s %-6s %-30s\n" "------" "-----" "------------" "------" "------------------------------"
        ((lines_to_clear+=2))
        
        for config in "${TEST_CONFIGS[@]}"; do
            read -r version threads <<< "$config"
            local key="${version}-${threads}"
            local job_id="${job_ids[$key]:-}"
            local status="${job_status[$key]:-}"
            
            if [ "$status" = "SKIPPED" ]; then
                printf "%-6s %-6s %-12s %-6s %-30s\n" "$version" "$threads" "-" "-" "SKIPPED"
                ((lines_to_clear++))
                continue
            fi
            
            if [ -z "$job_id" ]; then
                printf "%-6s %-6s %-12s %-6s %-30s\n" "$version" "$threads" "-" "-" "ERROR"
                ((lines_to_clear++))
                all_done=false
                continue
            fi
            
            local squeue_output
            squeue_output=$(squeue -j "$job_id" -h 2>/dev/null)
            
            if [ -z "$squeue_output" ]; then
                local output_file="${OUTPUT_DIR}/${version}-${threads}.out"
                if [ -f "$output_file" ]; then
                    job_status["$key"]="DONE"
                    printf "%-6s %-6s %-12s %-6s %-30s\n" "$version" "$threads" "$job_id" "-" "DONE"
                else
                    printf "%-6s %-6s %-12s %-6s %-30s\n" "$version" "$threads" "$job_id" "-" "ERROR"
                    all_done=false
                fi
            else
                local time_col reason_col
                time_col=$(squeue -j "$job_id" -h -o "%M" 2>/dev/null)
                reason_col=$(squeue -j "$job_id" -h -o "%R" 2>/dev/null)
                if [ -z "$reason_col" ]; then
                    reason_col=$(echo "$squeue_output" | awk '{for(i=8;i<=NF;i++) printf "%s ", $i; print ""}' | sed 's/[[:space:]]*$//')
                fi
                printf "%-6s %-6s %-12s %-6s %-30s\n" "$version" "$threads" "$job_id" "$time_col" "$reason_col"
                all_done=false
            fi
            ((lines_to_clear++))
        done
        
        if [ "$all_done" = true ]; then
            break
        fi
        
        sleep 3
        
        for ((i=0; i<lines_to_clear; i++)); do
            printf "\033[1A\033[K"
        done
    done
    
    echo ""
    echo "All jobs completed!"
}

extract_statistics() {
    ensure_directories
    echo "Extracting statistics from output files..."
    
    local results_file="${RESULTS_DIR}/results.csv"
    local header="sana_version,threads,output_file"
    for label in "${DETECTION_ORDER[@]}"; do
        header="${header},${label}"
    done
    echo "$header" > "$results_file"
    
    local count=0
    for output_file in "$OUTPUT_DIR"/*.out; do
        [ -f "$output_file" ] || continue
        local basename_file=$(basename "$output_file")
        
        [[ "$basename_file" =~ ([0-9]+\.[0-9]+)-([0-9]+)\.out ]] || continue
        local sana_version="${BASH_REMATCH[1]}"
        local threads="${BASH_REMATCH[2]}"
        
        local content
        content=$(cat "$output_file")
        
        local row="${sana_version},${threads},${basename_file}"
        for label in "${DETECTION_ORDER[@]}"; do
            local pattern="${DETECTIONS[$label]}"
            if [[ "$content" =~ $pattern ]]; then
                row="${row},${BASH_REMATCH[1]}"
            else
                row="${row},"
            fi
        done
        
        echo "$row" >> "$results_file"
        ((count++)) || true
    done
    
    if [ $count -eq 0 ]; then
        rm -f "$results_file"
        echo "No results to write"
    else
        echo "Extracted statistics from $count files"
        echo "Results written to: $results_file"
    fi
}

graph_results() {
    ensure_directories
    python3 "$SCRIPT_DIR/graph.py"
}

main() {
    local task="${1:-}"
    
    case "$task" in
        compute)
            submit_jobs
            ;;
        extract)
            extract_statistics
            ;;
        graph)
            graph_results
            ;;
        clear)
            clear_output
            ;;
        *)
            echo "Usage: $0 {compute|extract|graph|clear}" >&2
            echo ""
            echo "Tasks:"
            echo "  compute  - Submit jobs and wait for completion"
            echo "  extract  - Parse results from output files"
            echo "  graph    - Visualize results (not implemented)"
            echo "  clear    - Empty output directory"
            exit 1
            ;;
    esac
}

if [ "${BASH_SOURCE[0]}" = "${0}" ]; then
    [ $# -eq 0 ] && (main "help"; exit 1)
    [ "$1" = "--task" ] && [ $# -lt 2 ] && (main "help"; exit 1)
    main "${2:-$1}"
fi

