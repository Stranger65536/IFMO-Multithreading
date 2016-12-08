@ECHO OFF
@REM 10 100 500 1000 2000 3000 4000 5000 6000 7000 8000 9000 10000 20000 30000 40000 50000 60000 70000 80000 90000 100000 200000 300000 400000 500000 600000 700000 800000 900000 1000000 2000000 3000000 4000000 5000000 6000000 7000000 8000000 9000000 10000000 20000000 30000000 40000000 50000000 60000000 
for %%i in (70000000 80000000 90000000 100000000) do (
    echo size = %%i
    echo 1 host
    for /L %%j in (1,1,10) do (
        echo iteration %%j
        mpiexec -n 1 x64\Release\Lab03.exe input.txt -o out.txt -g %%i
        python is_sorted.py out.txt
    )
    echo 2 hosts
    for /L %%j in (1,1,10) do (
        echo iteration %%j 
        mpiexec -n 2 x64\Release\Lab03.exe input.txt -o out.txt -g %%i   
        python is_sorted.py out.txt
    )
    echo 3 hosts
    for /L %%j in (1,1,10) do (
        echo iteration %%j
        mpiexec -n 3 x64\Release\Lab03.exe input.txt -o out.txt -g %%i       
        python is_sorted.py out.txt
    )
    echo 4 hosts
    for /L %%j in (1,1,10) do (
        echo iteration %%j
        mpiexec -n 4 x64\Release\Lab03.exe input.txt -o out.txt -g %%i       
        python is_sorted.py out.txt
    )
)
@PAUSE