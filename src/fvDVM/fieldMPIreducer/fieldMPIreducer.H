#ifndef FIELDMPIREDUCER_H
#define FIELDMPIREDUCER_H
#define fvDVM_H
#include <mpi.h>
#include "fvCFD.H"

class fieldMPIreducer
{
private:
    label pd;
    //-  number of MPI process
    label nproc_;
    //-  own MPI rank
    label rank_;

    //each column is a communicator, has the whole velocity space
    label col_size;
    label col_rank;
    MPI_Comm vel_comm;

    //-  user defined MPI_Operator
    MPI_Op opSumVec_;
    //-  user defined MPI_Datatype for OpenFOAM vector data
    MPI_Datatype vecType_;
    //-  user defined MPI_Operator
    MPI_Op opSumTensor_;
    //-  user defined MPI_Datatype for OpenFOAM vector data
    MPI_Datatype tensorType_;

    //- dvParallel ?
    bool dvParallel_;

public:
    //- Constructors
    fieldMPIreducer(Foam::argList &args, int *argc, char ***argv);
    //- Destructor
    ~fieldMPIreducer();

    //- Memper functions

    //- MPI trnasfer
    void reduceField(volScalarField &vsf);
    void reduceField(volVectorField &vvf);
    void reduceField(surfaceVectorField &svf);
    void reduceField(surfaceScalarField &ssf);
    void reduceField(Field<scalar> &sf);
    void reduceField(Field<vector> &vf);
    void reduceField(Field<tensor> &vf);

    void spilt_comm(MPI_Comm &vel_comm);

    //- Access
    inline int nproc() const { return nproc_; };
    inline int rank() const { return rank_; };
    inline int csize() const { return col_size; };
    inline int crank() const { return col_rank; };
    inline MPI_Comm getcomm() const { return vel_comm; };
    inline bool dvParallel() const {return dvParallel_;};
    inline int npd() const {return pd;};
    //- Static member function
    static void vectorSum(vector *in, vector *inout, int *len, MPI_Datatype *dptr);
    static void tensorSum(tensor *in, tensor *inout, int *len, MPI_Datatype *dptr);
};
#endif
