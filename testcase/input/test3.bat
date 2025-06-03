REM Chạy countdownUI ở foreground
echo ==== 1. Running countdownUI in foreground:  ====
countdownUI10.exe
echo "Foreground execution completed!"

REM Chạy countdownUI ở background
echo    
echo ==== 2. Running countdownUI in background:  ====
countdownUI10.exe &
echo "Background execution initiated!"

REM Hoàn tất
echo ==== END ====