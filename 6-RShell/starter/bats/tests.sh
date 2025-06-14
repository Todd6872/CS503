#!/usr/bin/env bats

############################ DO NOT EDIT THIS FILE #####################################
# File: assignement_tests.sh
# 
# DO NOT EDIT THIS FILE
#
# Add/Edit Student tests in student_tests.sh
# 
# All tests in this file must pass - it is used as part of grading!
########################################################################################

# Helper function to check if output contains expected text
# Usage: assert_output_contains "expected text"
assert_output_contains() {
    local expected="$1"
    if [[ ! "$output" =~ "$expected" ]]; then
        echo "Expected output to contain: '$expected'"
        echo "Got output:"
        echo "$output"
        return 1
    fi
}

@test "Client-Server Pipes" {
    run ./dsh -s -i 0.0.0.0 -p 7890 &
    sleep 1 &
    sleep 1 &
    sleep 1 &
    sleep 1 &
    run ./dsh -c -i 0.0.0.0 -p 7890 <<EOF              
ls -l | grep dshlib.c
EOF

    # Verify key elements in the output
    assert_output_contains "dshlib.c"
    assert_output_contains "cmd loop returned 0"
    
    # Verify the command executed successfully
    [ "$status" -eq 0 ]
}

@test "Client-Server uname" {
    run ./dsh -s -i 0.0.0.0 -p 7890 &
    sleep 1 &
    sleep 1 &
    sleep 1 &
    sleep 1 &
    run ./dsh -c -i 0.0.0.0 -p 7890 <<EOF              
uname -a
EOF

    # Verify key elements in the output
    assert_output_contains "Linux"
    assert_output_contains "cmd loop returned 0"
    
    # Verify the command executed successfully
    [ "$status" -eq 0 ]
}

@test "Client-Server Echo" {
    run ./dsh -s -i 0.0.0.0 -p 7890 &
    sleep 1 &
    sleep 1 &
    sleep 1 &
    sleep 1 &
    run ./dsh -c -i 0.0.0.0 -p 7890 <<EOF              
echo " hello     world     "
EOF

    # Verify key elements in the output
    assert_output_contains " hello     world     "
    assert_output_contains "cmd loop returned 0"
    
    # Verify the command executed successfully
    [ "$status" -eq 0 ]
}

@test "Client-Server Which which ... which?" {
    run ./dsh -s -i 0.0.0.0 -p 7890 &
    sleep 1 &
    sleep 1 &
    sleep 1 &
    sleep 1 &
    run ./dsh -c -i 0.0.0.0 -p 7890 <<EOF                
which which
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    assert_output_contains "/usr/bin/which"
    assert_output_contains "cmd loop returned 0"

     # Verify the command executed successfully
    [ "$status" -eq 0 ]

}

@test "Client-Server find in file" {
    run ./dsh -s -i 0.0.0.0 -p 7890 &
    sleep 1 &
    sleep 1 &
    sleep 1 &
    sleep 1 &
    run ./dsh -c -i 0.0.0.0 -p 7890 <<EOF                
grep "exit(EXIT_FAILURE)" dshlib.c
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    assert_output_contains "exit(EXIT_FAILURE);"
    assert_output_contains "cmd loop returned 0"

     # Verify the command executed successfully
    [ "$status" -eq 0 ]

}

@test "Client-Server stop-server" {
    run ./dsh -s -i 0.0.0.0 -p 7890 &
    sleep 1 &
    sleep 1 &
    sleep 1 &
    sleep 1 &
    run ./dsh -c -i 0.0.0.0 -p 7890 <<EOF                
stop-server
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    assert_output_contains "server appeared to terminate - exiting"
    assert_output_contains "cmd loop returned -50"

     # Verify the command executed successfully
    [ "$status" -eq 0 ]

}

@test "Local-Command Pipes" {
    run "./dsh" <<EOF            
ls -l | grep dshlib.c
EOF

    # Verify key elements in the output
    assert_output_contains "dshlib.c"
    assert_output_contains "cmd loop returned 0"
    
    # Verify the command executed successfully
    [ "$status" -eq 0 ]
}

