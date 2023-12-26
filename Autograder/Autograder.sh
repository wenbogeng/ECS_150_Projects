#!/bin/bash


# Check Submission files, grading files, grading executables
missingFiles=false
for file in $(cat Config/SubmissionFiles.txt); do
    if [ ! -f $file ]; then
        echo "Missing submission file $file. Add it to this directory and re-run the script."
        missingFiles=true
    fi
done

for file in $(cat Config/RequiredGradingFiles.txt); do
    if [ ! -f $file ]; then
        echo "Missing grading file $file. Re-install the autograder and re-run the script."
        missingFiles=true
    fi
done

for exe in $(cat Config/SubmissionExecutables.txt); do
    if [ ! -f ${exe/exe/grader} ]; then
        echo "Missing grading executable $exe.grader. Re-install the autograder and re-run the script."
        missingFiles=true
    fi
#    rm -f $exe
done
#rm -f *.o

if [ "$missingFiles" = true ]; then
    echo "Some files are missing. Aborting..."
    exit
fi

# Try compiling
chmod u+x *.sh
echo "Trying to compile your submission..."
make
compileError=false
for file in $(cat Config/SubmissionExecutables.txt); do
    if [ ! -f $file ]; then
        echo "Your executable $file is missing after calling make."
        compileError=true
    fi
done

if [ "$compileError" = true ]; then
    echo "Errors encountered while compiling your program. Aborting..."
    exit
fi

# Run tests
rm -rf TestCaseSubmissions
mkdir TestCaseSubmissions
totalprojectscore=0
for exe in $(cat Config/SubmissionExecutables.txt); do
    numtests=$(ls TestCases/*${exe/.exe/}* | wc -l)
    cp $exe ${exe/.exe/}
    usage=Config/${exe/exe/}usage
    submissioncommand=$(cat $usage | head -n 1)
    submissionoutput=$(cat $usage | head -n 2 | tail -n 1)
    grader=$(cat $usage | head -n 3 | tail -n 1)    
    points=$(cat $usage | tail -n 1 | cut -f1 -d" ")
    totalexescore=0
    for i in $(seq 1 $numtests); do
        testfile="TestCases/TestCase$i.${exe/exe/}json"
        if [ ! -f $testfile ]; then
            echo "Error: missing test case $testfile. Re-copy all grading files and re-run the script."
            exit
        fi
        command="$submissioncommand > $submissionoutput"
        eval echo "Now grading: $submissioncommand"
        eval $command
        score=$(eval $grader)
        totalexescore=$(echo "$totalexescore + $score" | bc -l)
    done
    exegrade=$(echo "scale=2; $points * $totalexescore / (100 * $numtests)" | bc -l)
    totalprojectscore=$(echo "scale=2; $exegrade + $totalprojectscore" | bc -l)
    echo $exe grade: $exegrade of $points points possible
done

echo Your program received $totalprojectscore out of 100 possible points for the sample test cases. At least 70% of the test cases will be used during actual grading.
