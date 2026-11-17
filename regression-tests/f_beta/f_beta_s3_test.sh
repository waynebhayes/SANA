#!/bin/bash
REG_DIR=${REG_DIR:-regression-tests/f_beta}

# List of networks to process
networks=(
    "networks/RNorvegicus.el"
    "networks/CElegans.el"
    "networks/AThaliana.el"
    "networks/DMelanogaster.el"
    "networks/human/human.el"
    "networks/HSapiens/HSapiens.el"
)

# SANA command base
sana_command="${SANA_EXE:-./sana2.2} -f_beta 1,1 -fg1"
log_file="$REG_DIR/f_beta_s3_test.txt"

parse_sana_out() {
    local file=$1
    local s3=0.69
    local f_beta=0.0
    local s3_found=false

    while IFS= read -r line; do
        if [[ "$line" == "s3:"* && $s3_found == false ]]; then
            s3=$(echo "$line" | awk -F ':' '{print $2}' | xargs)
            s3_found=true
        elif [[ "$line" == *"f_beta:"* ]]; then
            f_beta=$(echo "$line" | awk -F ':' '{print $2}' | xargs)
        fi
    done < "$file"

    echo "$s3 $f_beta"
}

# Function to check monotonicity
check_monotonicity() {
    local pairs=("$@")  # Pass array by value
    local increasing=true
    local decreasing=true

    for ((k = 0; k < ${#pairs[@]} - 2; k+=2)); do
        ec1=${pairs[k]}
        fb1=${pairs[k+1]}
        ec2=${pairs[k+2]}
        fb2=${pairs[k+3]}

        # Compare to check monotonicity
        if (( $(echo "$ec1 <= $ec2" | bc -l) )); then
            if (( $(echo "$fb1 > $fb2" | bc -l) )); then
                increasing=false
            fi
        else
            increasing=false
        fi

        if (( $(echo "$ec1 >= $ec2" | bc -l) )); then
            if (( $(echo "$fb1 < $fb2" | bc -l) )); then
                decreasing=false
            fi
        else
            decreasing=false
        fi
    done

    if [[ $increasing == true || $decreasing == true ]]; then
        echo "Monotonic relationship detected."
    else
        echo "No monotonic relationship detected."
    fi
}

die() {
    echo "$1" >&2
    exit 1
}

results_list=()  # Initialize results_list

# Process networks sequentially in pairs (1 & 2, 2 & 3, etc.)
for ((i = 0; i < ${#networks[@]} - 1; i++)); do
    network1=${networks[$i]}
    network2=${networks[$i+1]}
    out_file="$REG_DIR/fbeta-f1-${network1##*/}-${network2##*/}"

    echo "Processing: $network1 and $network2" | tee -a "$log_file"
    command="$sana_command $network1 -fg2 $network2 -tolerance 0 -seed 1 -it 1 -o $out_file > /dev/null"
    eval "$command"
    
    PARA_STATUS=$?
    if [[ $PARA_STATUS -ne 0 || ! -f "$out_file.out" ]]; then
        echo "Error: Failed for $network1 and $network2" | tee -a "$log_file" >&2
        exit 1  # Exit with failure code
    fi

    # Parse outfile
    metrics=$(parse_sana_out "$out_file.out")
    s3=$(echo "$metrics" | awk '{print $1}')
    f_beta=$(echo "$metrics" | awk '{print $2}')

    # Store results for monotonicity check
    results_list+=("$s3" "$f_beta")

    # Print the results
    if [[ -n "$s3" && -n "$f_beta" ]]; then
        echo "Network Pair: $network1 and $network2" | tee -a "$log_file"
        echo "s3: $s3" | tee -a "$log_file"
        echo "f_beta: $f_beta" | tee -a "$log_file"
        echo "" | tee -a "$log_file"
    else
        die "F-beta/S3 output check failed for $network1 and $network2"
    fi

    rm -f "$out_file.out"
done

# Check monotonicity after processing
if [[ ${#results_list[@]} -gt 2 ]]; then
    echo "Analyzing monotonicity..." | tee -a "$log_file"
    check_monotonicity "${results_list[@]}" | tee -a "$log_file"
else
    echo "Insufficient data for monotonicity analysis." | tee -a "$log_file"
fi

# Final report
echo "Processing completed." | tee -a "$log_file"
