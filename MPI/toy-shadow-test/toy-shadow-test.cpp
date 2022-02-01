#include <mpi.h>
#include <random>
#include <assert.h>
#include <unistd.h>

// run the following commands on odin to import MPI and increase memory limit
// module load mpi/openmpi3-x86_64
// ulimit -l 65536

// mpirun -np (number of processes) toy-shadow-test

// NOTE: The point of this program is to demonstrate that we can have
// the shadow network updated dynamically without barrier synchronization.
// So here we have thread 0 is the "shadow" thread and worker_threads are
// sending increment and decrement ops into an array that is initually all zeros.

// So in principle the sum across all array elements should be zero; it'll have a
// bit of error since we are NOT synchronizing, and this program shows that the
// sum is never far from zero. Furthermore, if you add barrier sync, then the
// program slows down immensely. Since it's more important for SANA to be fast
// than to have the exact shadow network at every instant, we opt to allow the
// array to have non-zero sum in favor of speed.

#define ABS(a) ((a) < 0 ? -(a) : (a))

long arraySum(long *array, int s)
{
    long final_sum = 0;
    for (int i = 0; i < s; ++i)
    {
        final_sum = final_sum + array[i];
    }
    return final_sum;
}

int main(int argc, char **argv)
{
    // Initial
    srand(time(NULL));

    int size, rank;
    int arraySize, localArraySize;
    long *array, *localarray;
    int *model;

    // test variables (change)
    arraySize = 1000000;
    const int numberSends = 1000000;
    const int printThresh = 0;
    const bool verbose = false;
    const int sumInterval = 10000;

    localArraySize = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (size < 2)
    {
        printf("This program needs to be run with at least 2 processes.\n");
        MPI_Finalize();
        return 0;
    }

    // Create the window
    if (rank == 0)
    {
        printf("Initialised.\n");
        localArraySize = arraySize;
    }
    MPI_Win window;
    MPI_Win_allocate_shared(localArraySize * sizeof(long), sizeof(long), MPI_INFO_NULL, MPI_COMM_WORLD, &localarray, &window);
    printf("[MPI process %d] Window created.\n", rank);
    int flag;
    MPI_Win_get_attr(window, MPI_WIN_MODEL, &model, &flag);

    if (1 != flag)
    {
        printf("Attribute MPI_WIN_MODEL not defined\n");
    }
    else
    {
        if (MPI_WIN_UNIFIED == *model)
        {
            if (rank == 0)
                printf("Memory model is MPI_WIN_UNIFIED\n");
        }
        else
        {
            if (rank == 0)
                printf("Memory model is *not* MPI_WIN_UNIFIED\n");

            MPI_Finalize();
            return 1;
        }
    }

    array = localarray;

    MPI_Aint winsize;
    int windisp;
    int *winptr;

    if (rank != 0)
    {

        MPI_Win_shared_query(window, 0, &winsize, &windisp, &array);
    }

    // initialise table on rank 0
    if (rank == 0)
    {
        for (int i = 0; i < arraySize; i++)
        {
            array[i] = 0;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0)
    {
        printf("FIRST BARRIER.\n");
    }

    if (rank == 0)
    {
        printf("Process 0 id: %ld %ld\n", (long)getpid(), (long)getppid());
        if (verbose)
        {
            std::cout << "Process 0: array contains : ";
            for (int i = 0; i < arraySize; ++i)
            {
                std::cout << array[i] << " ";
            }
            std::cout << std::endl;
        }

        // update array
        int finished = 0;
        while (finished < size - 1)
        {
            int x;
            MPI_Status status;
            MPI_Recv(&x, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == 1)
            {
                array[x]--;
            }
            else if (status.MPI_TAG == 0)
            {
                array[x]++;
            }
            else
            {
                finished++;
            }
        }
        //MPI_Win_fence(0, window);
        //MPI_Barrier(MPI_COMM_WORLD);
        if (verbose)
        {
            std::cout << "Process 0: Array after receives : ";
        }

        int final_sum = 0;
        for (int i = 0; i < arraySize; ++i)
        {
            if (verbose)
            {
                std::cout << array[i] << " ";
            }

            final_sum = final_sum + array[i];
        }
        if (verbose)
        {
            std::cout << std::endl;
        }

        printf("Rank 0: Final sum is %d\n", final_sum);
        //assert(final_sum == arraySize * (size - 1));
    }
    else
    {
        printf("[MPI process %d] sending increments.\n", rank);
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> gen(0, arraySize - 1); // distribution in range [1, 6]

        int x;
        for (int i = 0; i < numberSends; ++i)
        {
            // decrement tag = 1
            x = gen(rng);
            MPI_Send(&x, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            // increment tag = 0
            x = gen(rng);
            MPI_Send(&x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

            if (i % sumInterval == 0)
            {
                //MPI_Barrier(MPI_COMM_WORLD);
                long sum = arraySum(array, arraySize);
                if (ABS(sum) > printThresh)
                {
                    printf("Process %d iter %d: Sum %d\n", rank, i, sum);
                }
            }
        }
        MPI_Send(&x, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);

        //printf("\nProcess %d: Array after receives : ", rank);
        int final_sum = arraySum(array, arraySize);
        if (final_sum != 0)
        {
            printf("Rank %d: Final sum is %d\n", rank, final_sum);
        }

        //assert(final_sum == 0);
    }

    // Finalisation
    MPI_Barrier(MPI_COMM_WORLD);
    int final_sum = arraySum(array, arraySize);
    assert(final_sum == 0);
    if (rank == 0)
    {
        printf("FINAL BARRIER -> Finalisation.\n");
    }

    MPI_Finalize();
}

// mpirun -np 11 main 1> output.txt

// use long, have decrement message,
// array has signed integers
// sends increment and decrement every message
// sum should remain 0

// every millionth message, read through array and print sum
// every minute, every time to start
// every thread print sum, and sum of absolute values
// close to 0, ???

// message id array size 10 counts how many messages it has processed.

// check how big message queue is, is there a function?
// run for 30 minutes, find a way to find shadow thread not overwhelmed.
// rank 0 does 10x work, should throttle

// extra shared array that counts messages received

// https://pages.tacc.utexas.edu/~eijkhout/pcse/html/mpi-shared.html