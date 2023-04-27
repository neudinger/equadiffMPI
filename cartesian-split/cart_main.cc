#include <stdio.h>
#include <mpi.h>
#include <iostream>
#include <sstream>
#include <vector>

// 199711L(until C++11),
// 201103L(C++11),
// 201402L(C++14),
// 201703L(C++17), or
// 202002L(C++20)
#if __cplusplus < 201703L
#define typeof __typeof__
#endif

#define XORSWAP_UNSAFE(a, b) \
    ((a) ^= (b), (b) ^= (a), \
     (a) ^= (b))
#define XORSWAP(a, b)                                        \
    ((&(a) == &(b)) ? (a) /* Check for distinct addresses */ \
                    : XORSWAP_UNSAFE(a, b))
// replaced by std::swap
#define SWAP(x, y)          \
    {                       \
        typeof(x) SWAP = x; \
        x = y;              \
        y = SWAP;           \
    }

// row-major order
// https://stackoverflow.com/questions/33862730/row-major-vs-column-major-confusion
#define ROW_MAJOR_IDX(i, j) ((i) * (block_size_y + 2)) + (j)

// #define NUM_ROWS 1024
// #define NUM_COLS 1024
// int ROW_MAJOR [NUM_ROWS][NUM_COLS];
// int i=0, j=0;
// for(i; i<NUM_ROWS; i++){
//         for(j; j<NUM_COLS; j++){
//                 ROW_MAJOR[i][j]=(i+j);//NOTE i,j order here!
//         }//end inner for
// }//end outer fo

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int namelen, comm_size, my_rank;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &my_rank);
    MPI_Comm_size(comm, &comm_size);
    MPI_Get_processor_name(processor_name, &namelen);
    std::size_t nxn, energy, niters;

    if (my_rank == 0)
    {
        // argument checking
        if (argc < 4)
        {
            if (!my_rank)
                printf("usage: stencil_mpi <nxn> <energy> <niters>\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        nxn = std::stoul(argv[1]);    // nxn grid
        energy = std::stoul(argv[2]); // energy to be injected per iteration
        niters = std::stoul(argv[3]); // number of iterations

        // distribute arguments
        std::size_t args[3] = {nxn, energy, niters};
        MPI_Bcast(args, 3, MPI_UNSIGNED_LONG, 0, comm);
    }
    else
    {
        std::size_t args[3];
        MPI_Bcast(args, 3, MPI_UNSIGNED_LONG, 0, comm);
        nxn = args[0];
        energy = args[1];
        niters = args[2];
    }

    int pdims[2] = {0, 0};
    // compute good (rectangular) domain decomposition
    MPI_Dims_create(comm_size, 2, pdims);
    std::size_t MPI_dims_px = pdims[0];
    std::size_t MPI_dims_py = pdims[1];
    // create Cartesian topology
    const int periods[2] = {false, false};

    MPI_Comm cart_comm;
    MPI_Cart_create(comm, 2, pdims, periods, 0, &cart_comm);

    // get my local x,y coordinates
    int coords[2];
    MPI_Cart_coords(cart_comm, my_rank, 2, coords);
    std::size_t MPI_coords_x = coords[0];
    std::size_t MPI_coords_y = coords[1];

    // my_rank
    // determine my four neighbors
    int north, south, east, west;
    MPI_Cart_shift(cart_comm, 1, 1, &west, &east);
    MPI_Cart_shift(cart_comm, 0, 1, &north, &south);
    MPI_Comm_rank(cart_comm, &my_rank);

    // decompose the domain
    std::size_t block_size_x = nxn / MPI_dims_px;       // block size in x
    std::size_t block_size_y = nxn / MPI_dims_py;       // block size in y
    std::size_t offset_x = MPI_coords_x * block_size_x; // offset in x
    std::size_t offset_y = MPI_coords_y * block_size_y; // offset in y
    MPI_Win windows[2];
    double *windows_buffer[2];

    MPI_Alloc_mem((block_size_x + 2) * (block_size_y + 2) * sizeof(double), MPI_INFO_NULL, &windows_buffer[0]); // 1-wide halo zones!
    MPI_Alloc_mem((block_size_x + 2) * (block_size_y + 2) * sizeof(double), MPI_INFO_NULL, &windows_buffer[1]); // 1-wide halo zones!

    MPI_Win_create(windows_buffer[0], (block_size_x + 2) * (block_size_y + 2), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &windows[0]);
    MPI_Win_create(windows_buffer[1], (block_size_x + 2) * (block_size_y + 2), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &windows[1]);
    MPI_Barrier(MPI_COMM_WORLD);

    // create north-south datatype
    MPI_Datatype north_south_type, east_west_type;
    MPI_Type_contiguous(block_size_y, MPI_DOUBLE, &north_south_type);
    MPI_Type_commit(&north_south_type);
    // create east-west type
    MPI_Type_vector(/* int count = */ block_size_x + 2,
                    /* int blocklength = */ 1,
                    /* int stride = */ block_size_y + 2,
                    /* MPI_Datatype oldtype = */ MPI_DOUBLE,
                    /* MPI_Datatype *newtype = */ &east_west_type);
    MPI_Type_commit(&east_west_type);

    MPI_File fh;
    MPI_Status status;
    MPI_File_open(MPI_COMM_SELF, ("rank_" + std::to_string(my_rank) + ".txt").c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

    std::stringstream stream_message;

    stream_message << "my_rank = " << my_rank << "\n"
                   << "block_size_x = " << block_size_x << "\n"
                   << "block_size_y = " << block_size_y << "\n"
                   << "offset_x = " << offset_x << "\n"
                   << "offset_y = " << offset_y << "\n"
                   << "[MPI process " << my_rank << "] I am located at (" << MPI_coords_x << ", " << MPI_coords_y << ").\n"
                   << "Dims given (" << pdims[0] << ", " << pdims[1] << ")\n"
                   << "Periods given (" << periods[0] << ", " << periods[1] << ")\n"
                   << "north = " << north << "\n"
                   << "south = " << south << "\n"
                   << "east = " << east << "\n"
                   << "west = " << west << "\n"
                   << std::endl;
    for (size_t i = 0; i < (block_size_x + 2) * (block_size_y + 2); ++i)
        (*windows_buffer)[i] = 0;

    if (offset_x == 0)
        for (size_t j = 1; j < block_size_y + 1; j++)
            (*windows_buffer)[ROW_MAJOR_IDX(1, j)] = (double)energy;

    for (size_t i = 0; i < block_size_x + 2; ++i)
    {
        for (size_t j = 0; j < (block_size_y + 2); ++j)
            stream_message << (*windows_buffer)[ROW_MAJOR_IDX(i, j)] << "\t";
        stream_message << std::endl;
    }
    stream_message << std::endl;
    for (size_t i = 1; i < (block_size_x + 1); ++i)
    {
        for (size_t j = 1; j < (block_size_y + 1); ++j)
            stream_message << (*windows_buffer)[ROW_MAJOR_IDX(i, j)] << "\t";
        stream_message << std::endl;
    }
    stream_message << std::endl;
    MPI_Win_fence(MPI_MODE_NOPRECEDE, windows[0]);
    MPI_Win_fence(MPI_MODE_NOPRECEDE, windows[1]);
    MPI_Request reqs[4];
    for (size_t nb = 0; nb < niters; nb++)
    {
        MPI_Rget(
            /* void *origin_addr = */ &((*windows_buffer)[1]),
            /* int origin_count = */ 1,
            /* MPI_Datatype origin_datatype = */ north_south_type,
            /* int target_rank = */ north,
            /* MPI_Aint target_disp = */ (block_size_y + 2) * block_size_x + 1,
            /* int target_count = */ 1,
            /* MPI_Datatype target_datatype = */ north_south_type,
            /* MPI_Win win = */ *windows,
            /* MPI_Request *request = */ &reqs[0]);
        MPI_Rget(
            /* void *origin_addr = */ &((*windows_buffer)[(block_size_y + 2) * (block_size_x + 1) + 1]),
            /* int origin_count = */ 1,
            /* MPI_Datatype origin_datatype = */ north_south_type,
            /* int target_rank = */ south,
            /* MPI_Aint target_disp = */ (block_size_y + 2) + 1,
            /* int target_count = */ 1,
            /* MPI_Datatype target_datatype = */ north_south_type,
            /* MPI_Win win = */ *windows,
            /* MPI_Request *request = */ &reqs[1]);
        MPI_Rget(
            /* void *origin_addr = */ &((*windows_buffer)[block_size_y + 1]),
            /* int origin_count = */ 1,
            /* MPI_Datatype origin_datatype = */ east_west_type,
            /* int target_rank = */ east,
            /* MPI_Aint target_disp = */ 1,
            /* int target_count = */ 1,
            /* MPI_Datatype target_datatype = */ east_west_type,
            /* MPI_Win win = */ *windows,
            /* MPI_Request *request = */ &reqs[2]);
        MPI_Rget(
            /* void *origin_addr = */ &((*windows_buffer)[0]),
            /* int origin_count = */ 1,
            /* MPI_Datatype origin_datatype = */ east_west_type,
            /* int target_rank = */ west,
            /* MPI_Aint target_disp = */ block_size_y,
            /* int target_count = */ 1,
            /* MPI_Datatype target_datatype = */ east_west_type,
            /* MPI_Win win = */ *windows,
            /* MPI_Request *request = */ &reqs[3]);
        MPI_Waitall(4, reqs, MPI_STATUS_IGNORE);
        // ----------------
        MPI_Win_fence(0, windows[0]);
        MPI_Win_fence(0, windows[1]);
        // MPI_Barrier(MPI_COMM_WORLD);
        // ----------------
        
        for (std::size_t i = 1; i < (block_size_x + 1); ++i)
        {
            for (std::size_t j = 1; j < (block_size_y + 1); ++j)
            {
                (windows_buffer[1])[ROW_MAJOR_IDX(i, j)] =
                    0.25 * ((*windows_buffer)[ROW_MAJOR_IDX(i, j)] +
                            (*windows_buffer)[ROW_MAJOR_IDX(i + 1, j)] +
                            (*windows_buffer)[ROW_MAJOR_IDX(i - 1, j)] +
                            (*windows_buffer)[ROW_MAJOR_IDX(i, j + 1)] +
                            (*windows_buffer)[ROW_MAJOR_IDX(i, j - 1)]);
            }
        }

        for (std::size_t i = 0; i < block_size_x + 2; ++i)
        {
            for (std::size_t j = 0; j < (block_size_y + 2); ++j)
                stream_message << (*windows_buffer)[ROW_MAJOR_IDX(i, j)] << "\t";
            stream_message << std::endl;
        }
        stream_message << std::endl;
        for (std::size_t i = 1; i < (block_size_x + 1); ++i)
        {
            for (std::size_t j = 1; j < (block_size_y + 1); ++j)
                stream_message << (*windows_buffer)[ROW_MAJOR_IDX(i, j)] << "\t";
            stream_message << std::endl;
        }
        stream_message << std::endl;
        std::swap(windows_buffer[0], windows_buffer[1]);
        std::swap(windows[0], windows[1]);
    }
    std::string message = stream_message.str();
    MPI_File_write(fh, message.c_str(), message.length(), MPI_CHAR, &status);
    MPI_File_close(&fh);
    MPI_Win_fence(MPI_MODE_NOSUCCEED, windows[0]);
    MPI_Win_fence(MPI_MODE_NOSUCCEED, windows[1]);
    MPI_Free_mem(windows_buffer[0]);
    MPI_Free_mem(windows_buffer[1]);
    MPI_Win_free(&windows[0]);
    MPI_Win_free(&windows[1]);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
