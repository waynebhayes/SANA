#!/bin/bash
REG_DIR=${REG_DIR:-f_beta_regression_tests}

# List of networks to process
networks=(
    "networks/RNorvegicus.el"
    "networks/CElegans.el"
    "networks/AThaliana.el"
    "networks/DMelanogaster.el"
    "networks/human/human.el"
    "networks/HSapiens/HSapiens.el"
)

# Output log file
log_file="$REG_DIR/f_beta_ics_test.txt"

# SANA command base
sana_command="${SANA_EXE:-./sana2.2} -ec 0.75 -f_beta 0.25,inf -fg1"

# Clean previous log
> "$log_file"

# Function to parse outfile
parse_sana_out() {
    local file=$1
    local ics=0.0
    local f_beta=0.0

    while IFS= read -r line; do
        if [[ "$line" == *"ics:"* ]]; then
            ics=$(echo "$line" | awk -F ':' '{print $2}' | xargs)
        elif [[ "$line" == *"f_beta:"* ]]; then
            f_beta=$(echo "$line" | awk -F ':' '{print $2}' | xargs)
        fi
    done < "$file"

    echo "$ics $f_beta"
}

die() {
    echo "$1" >&2
    echo "$1" >> "$log_file"
    exit 1
}

# Process consecutive pairs of networks
for ((i = 0; i < ${#networks[@]} - 1; i++)); do
    network1=${networks[$i]}
    network2=${networks[$i+1]}
    out_file="$REG_DIR/fbeta-ics-${network1##*/}-${network2##*/}"

    # Build and run the SANA command
    echo "Processing: $network1 and $network2" | tee -a "$log_file"
    command="$sana_command $network1 -fg2 $network2 -tolerance 0 -seed 1 -it 1 -o $out_file > /dev/null"
    eval "$command"
    
    PARA_STATUS=$?
    # Check if PARA_STATUS indicates an error
    if [[ $PARA_STATUS -ne 0 ]]; then
        die "Error: Command failed for $network1 and $network2 with status $PARA_STATUS"
    fi

    # Check for outfile
    if [[ ! -f "$out_file.out" ]]; then
        die "Error: outfile '$out_file.out' was not generated for $network1 and $network2"
    fi

    # Parse outfile
    metrics=$(parse_sana_out "$out_file.out")
    ics=$(echo "$metrics" | awk '{print $1}')
    f_beta=$(echo "$metrics" | awk '{print $2}')

    # Check if `ics` and `f_beta` are equal
    if [[ "$ics" != "$f_beta" ]]; then
        die "Failure for $network1 and $network2: ICS: $ics, F-beta: $f_beta"
    fi

    # Save results to log file
    {
        echo "Network Pair: $network1 and $network2"
        echo "ics: $ics"
        echo "f_beta: $f_beta"
        echo ""
    } >> "$log_file"

    # Remove outfile
    rm -f "$out_file.out"
done

# Final report
echo "Processing completed." | tee -a "$log_file"
