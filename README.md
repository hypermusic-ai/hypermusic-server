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

## 📚 API Documentation

This section describes the available API endpoints for the server backend.

> ⚠️ All protected routes require authentication via a secure `access_token` cookie. After login, this cookie is automatically included in requests by the browser.

---

### 📄 Interface

#### `OPTIONS /`

Returns CORS headers for the simple HTML interface endpoint.

- **Authentication**: ❌ Public

---

#### `GET /`

Returns the simple HTML interface to interact with the server.

- **Response Type**: `text/html`  
- **Authentication**: ❌ Public

---

### 🔐 Authentication

#### `GET /nonce/<address>`

Returns a nonce for the specified Ethereum address to use in the login message.

- **Params**:
  - `address`: Ethereum address (e.g., `0x123...`)
- **Response**:

```json
{ "nonce": "string" }
```

- **Authentication**: ❌ Public

#### `POST /auth`

Verifies the signed nonce and sets an authentication cookie (access_token).

- **Request Body(JSON)**:
  
```json
{
  "address": "0x...",
  "message": "Login nonce: ...",
  "signature": "0x..."
}
```

- **Response**: `200 OK` : `Authentication successful`

- **Authentication**:
  - **Header**: Set-Cookie: `access_token`=...; HttpOnly; Secure; SameSite=Strict; Path=/
  - **Header**: Set-Cookie: `refresh_token`=...; HttpOnly; Secure; SameSite=Strict; Path=/refresh"

- **Authentication**: ❌ Public

#### `POST /refresh`

Verifies `refresh_token` and `access_token` and generates new `access_token`.

- **Response**: `200 OK` : `Authentication successful`

- **Authentication**:
  - **Header**: Set-Cookie: `access_token`=...; HttpOnly; Secure; SameSite=Strict; Path=/
  - **Header**: Set-Cookie: `refresh_token`=...; HttpOnly; Secure; SameSite=Strict; Path=/refresh"

- **Authentication**: ✅ Required

---

### 🧩 Features

#### `OPTIONS /feature`

Returns CORS headers for the feature endpoints.

#### `GET /feature/<name>/<ver?>`

Fetches a feature object by name and version.
If version is not given, returns newest feature of given name.

- **Params**:
  - `name`: String identifier
  - `ver?`: Optional String identifier

- **Response**: Feature data `(JSON)`

  ```json
  {
    "name": "...",
    "dimensions": [...]
  }
  ```

- **Authentication**: ❌ Public

#### `POST /feature`

Creates a new feature entry.

- **Request Headers**: `Cookie` must include `access_token`

- **Request Body**: `JSON` payload describing the feature
  
  ```json
  {
    "name": "...",
    "dimensions": [...]
  }
  ```

- **Response**: `201 Created` or `error`
- **Response Body** :

  ```json
  {
    "name": "...",
    "version": "1823..."
  }
  ```

- **Authentication**: ✅ Required

---

### 🔄 Transformation

#### `OPTIONS /transformation`

Returns CORS headers for the transformation endpoints.

#### `GET /transformation/<name>/<ver?>`

Fetches a transformation by name and version.
If version is not given, returns newest transformation of given name.

- **Params**:
  - `name`: String identifier
  - `ver`: Optional String identifier

- **Response**: Transformation data `(JSON)`

  ```json
  {
    "name": "...",
    "sol_src": "..."
  }
  ```

- **Authentication**: ❌ Public

#### `POST /transformation`

Creates a new transformation record.

- **Request Headers**: `Cookie` must include `access_token`

- **Request Body**: `JSON` payload describing the transformation

  ```json
  {
    "name": "...",
    "sol_src": "..."
  }
  ```

- **Response**: `201 Created` or `error`
- **Response Body**:

  ```json
  {
    "name": "...",
    "version": "1823..."
  }
  ```

- **Authentication**: ✅ Required

---
📜 **License**:

👨‍💻 **Contributors**: [Sawyer](https://github.com/MisterSawyer)

💡 **Additional Notes**:
