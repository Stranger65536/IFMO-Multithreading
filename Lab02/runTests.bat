@ECHO OFF
for %%i in (10 100 200 400 800 1600 3200 4800 6400 8000) do (
    echo size = %%i
    echo 1 host
    for /L %%j in (1,1,10) do (
        echo iteration %%j
        mpiexec -hosts 1 192.168.1.51 C:\Lab02\x64\Release\Lab02.exe matrix.txt -o C:\Lab02\approximation.txt 0.000000000000001 100000 -g %%i
    )
    echo 2 hosts
    for /L %%j in (1,1,10) do (
        echo iteration %%j 
        mpiexec -hosts 2 192.168.1.51 192.168.1.52 C:\Lab02\x64\Release\Lab02.exe matrix.txt -o C:\Lab02\approximation.txt 0.000000000000001 100000 -g %%i   
    )
    echo 4 hosts
    for /L %%j in (1,1,10) do (
        echo iteration %%j
        mpiexec -hosts 4 192.168.1.51 192.168.1.52 192.168.1.53 192.168.1.54 C:\Lab02\x64\Release\Lab02.exe matrix.txt -o C:\Lab02\approximation.txt 0.000000000000001 100000 -g %%i       
    )
)
@PAUSE