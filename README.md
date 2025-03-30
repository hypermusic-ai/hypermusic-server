# 🚀 Hypermusic server

## 📦 Dependencies

This project requires the following libraries:

- [**Asio**](https://think-async.com/Asio/)
- [**spdlog**](https://github.com/gabime/spdlog)
- [**cURL**](https://curl.se/)
- [**Protobuf**](https://protobuf.dev/)
- [**evmone**](https://github.com/ethereum/evmone)

🔗 **[Dependencies repository](https://github.com/hypermusic-ai/hypermusic-server-deps)** – Prebuilt dependencies and setup instructions can be found here.

## ⚙️ Configuring the Project

To configure the project using CMake, run the following command:

```sh
cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_INSTALL_PREFIX=install -DHYPERMUSIC_DEPS_PATH="..."
```

- `-S .`: Specifies the source directory.
- `-B build`: Specifies the build directory.
- `-DBUILD_TESTING=ON`: Enables tests.
- `-DCMAKE_INSTALL_PREFIX=install`: Specifies install directory.
- `-DHYPERMUSIC_DEPS_PATH="..."`: Specifies the dependencies path.

## 🛠️ Building the Project (Debug Mode)

To build the project in **Debug Mode**, use:

```sh
cmake --build build --config Debug
```

This will compile the project with debugging enabled.

## 🛠️ Installing the Project (Debug Mode)

To install project in **Debug Mode**, use:

```sh
cmake --build build --config Debug --target install
```

This will install the project with debugging enabled.

## 🖥️ Start the server

```sh
./build/Debug/HypermusicServer.exe
```

---
📜 **License**:

👨‍💻 **Contributors**: [Sawyer](https://github.com/MisterSawyer)

💡 **Additional Notes**:
