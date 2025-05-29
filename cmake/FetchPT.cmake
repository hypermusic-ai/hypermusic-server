include(ExternalProject)

ExternalProject_Add(pt_download
    GIT_REPOSITORY https://github.com/hypermusic-ai/PT.git
    GIT_TAG main
    PREFIX          "${PT_REPO_PREFIX}"
    SOURCE_SUBDIR solidity
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)

set(PT_REPO_PREFIX "${PT_REPO_PREFIX}/src/pt_repo")