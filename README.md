# Tiny-shell

Dự án shell nhỏ viết bằng C/C++ sử dụng CMake để build trên Windows với MinGW.

---

## Yêu cầu

- Windows 10/11
- [MinGW-w64](https://www.mingw-w64.org/) (đảm bảo đã thêm `mingw32-make` và gcc/g++ vào PATH)
- [CMake](https://cmake.org/download/) (cài đặt và thêm vào PATH)

---

## Cài đặt môi trường (nếu chưa có)

1. **Cài MinGW-w64:**

   - Tải MinGW-w64 từ https://www.mingw-w64.org/downloads/
   - Cài đặt chọn kiến trúc phù hợp (x86_64)
   - Thêm thư mục `bin` của MinGW vào biến môi trường `PATH` (vd: `C:\mingw64\bin`)

2. **Cài CMake:**

   - Tải CMake từ https://cmake.org/download/
   - Cài đặt và tích chọn thêm CMake vào PATH

---

## Cấu trúc thư mục dự án

tiny-shell/
├── build/          # Thư mục chứa các file build (tự động tạo khi biên dịch)  
├── include/        # Chứa các file header (*.h)  
├── src/            # Chứa mã nguồn (*.c, *.cpp)  
│   └── main.cpp    # File chính chứa hàm main()  
├── CMakeLists.txt  # File cấu hình build sử dụng CMake  
└── README.md       # File hướng dẫn dự án (file này)


---

## Hướng dẫn build và chạy

1. Mở **PowerShell** hoặc **cmd**

2. Điều hướng đến thư mục gốc dự án:  
Ví dụ  
    ```powershell
    cd D:\Mhung\Project\tiny-shell
    ```

3. Xóa thư mục build cũ (nếu có):

    ```powershell
    Remove-Item -Recurse -Force build
    ```

4. Tạo thư mục build mới và chuyển vào đó:

    ```powershell
    mkdir build
    cd build
    ```

5. Cấu hình dự án với CMake và chọn MinGW Makefiles làm generator:

    ```powershell
    cmake -G "MinGW Makefiles" ..
    ```

    Nếu không có lỗi, CMake sẽ tạo các file Makefile trong thư mục `build`.

6. Biên dịch dự án:

    ```powershell
    mingw32-make
    ```

    Sau khi chạy thành công, file thực thi `myShell.exe` sẽ được tạo trong thư mục `build`.

7. Chạy chương trình:

    ```powershell
    ./myShell.exe
    ```

Chạy các lần sau thì chỉ cần chạy bước 6, 7 (nếu không sửa file CMakeLists.txt).