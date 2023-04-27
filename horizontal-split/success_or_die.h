#ifndef SUCCESS_OR_DIE_H
#define SUCCESS_OR_DIE_H

#include <mpi.h>
#include <stdlib.h>
#include <iostream>
#include <array>

static constexpr const std::array MPI_ERR_DEF{
    "MPI_SUCCESS",
    "MPI_ERR_BUFFER",
    "MPI_ERR_COUNT",
    "MPI_ERR_TYPE",
    "MPI_ERR_TAG",
    "MPI_ERR_COMM",
    "MPI_ERR_RANK",
    "MPI_ERR_REQUEST",
    "MPI_ERR_ROOT",
    "MPI_ERR_GROUP",
    "MPI_ERR_OP",
    "MPI_ERR_TOPOLOGY",
    "MPI_ERR_DIMS",
    "MPI_ERR_ARG",
    "MPI_ERR_UNKNOWN",
    "MPI_ERR_TRUNCATE",
    "MPI_ERR_OTHER",
    "MPI_ERR_INTERN",
    "MPI_ERR_IN_STATUS",
    "MPI_ERR_PENDING",
    "MPI_ERR_ACCESS",
    "MPI_ERR_AMODE",
    "MPI_ERR_ASSERT",
    "MPI_ERR_BAD_FILE",
    "MPI_ERR_BASE",
    "MPI_ERR_CONVERSION",
    "MPI_ERR_DISP",
    "MPI_ERR_DUP_DATAREP",
    "MPI_ERR_FILE_EXISTS",
    "MPI_ERR_FILE_IN_USE",
    "MPI_ERR_FILE",
    "MPI_ERR_INFO_KEY",
    "MPI_ERR_INFO_NOKEY",
    "MPI_ERR_INFO_VALUE",
    "MPI_ERR_INFO",
    "MPI_ERR_IO",
    "MPI_ERR_KEYVAL",
    "MPI_ERR_LOCKTYPE",
    "MPI_ERR_NAME",
    "MPI_ERR_NO_MEM",
    "MPI_ERR_NOT_SAME",
    "MPI_ERR_NO_SPACE",
    "MPI_ERR_NO_SUCH_FILE",
    "MPI_ERR_PORT",
    "MPI_ERR_QUOTA",
    "MPI_ERR_READ_ONLY",
    "MPI_ERR_RMA_CONFLICT",
    "MPI_ERR_RMA_SYNC",
    "MPI_ERR_SERVICE",
    "MPI_ERR_SIZE",
    "MPI_ERR_SPAWN",
    "MPI_ERR_UNSUPPORTED_DATAREP",
    "MPI_ERR_UNSUPPORTED_OPERATION",
    "MPI_ERR_WIN",
    "MPI_T_ERR_MEMORY",
    "MPI_T_ERR_NOT_INITIALIZED",
    "MPI_T_ERR_CANNOT_INIT",
    "MPI_T_ERR_INVALID_INDEX",
    "MPI_T_ERR_INVALID_ITEM",
    "MPI_T_ERR_INVALID_HANDLE",
    "MPI_T_ERR_OUT_OF_HANDLES",
    "MPI_T_ERR_OUT_OF_SESSIONS",
    "MPI_T_ERR_INVALID_SESSION",
    "MPI_T_ERR_CVAR_SET_NOT_NOW",
    "MPI_T_ERR_CVAR_SET_NEVER",
    "MPI_T_ERR_PVAR_NO_STARTSTOP",
    "MPI_T_ERR_PVAR_NO_WRITE",
    "MPI_T_ERR_PVAR_NO_ATOMIC",
    "MPI_ERR_RMA_RANGE",
    "MPI_ERR_RMA_ATTACH",
    "MPI_ERR_RMA_FLAVOR",
    "MPI_ERR_RMA_SHARED",
    "MPI_T_ERR_INVALID",
    "MPI_T_ERR_INVALID_NAME"};

#define SUCCESS_OR_DIE(f...)                                    \
  {                                                             \
    const int mpi_return = f;                                   \
    if (mpi_return not_eq MPI_SUCCESS)                          \
    {                                                           \
      std::cerr << "Error: '" << #f                             \
                << "' [" __FILE__ << ":" << __LINE__ << "] \t"  \
                << MPI_ERR_DEF[static_cast<size_t>(mpi_return)] \
                << std::endl;                                   \
      exit(MPI_Abort(MPI_COMM_WORLD, mpi_return));              \
    }                                                           \
  }

#endif /* SUCCESS_OR_DIE_H */
