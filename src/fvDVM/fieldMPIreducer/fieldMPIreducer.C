#include "foam_defs.h"
#include "fieldMPIreducer.H"

#if FOAM_MAJOR <= 3
#define BOUNDARY_FIELD_REF boundaryField()
#else
#define BOUNDARY_FIELD_REF boundaryFieldRef()
#endif

fieldMPIreducer::fieldMPIreducer(
    Foam::argList &args,
    int *argc,
    char ***argv) : pd(1), nproc_(1), rank_(0), col_size(1), col_rank(0), dvParallel_(false)
{

    args.optionReadIfPresent("pd", pd);
    if (args.optionFound("dvParallel"))
    {
        if(!args.optionFound("parallel")) {
            MPI_Init(argc, argv);
        }
        MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
        MPI_Comm_size(MPI_COMM_WORLD, &nproc_);
        //physical parallel
        if (pd == nproc_)
        {
            rank_ = 0;
            nproc_ = 1;
        }
        else
        {   
            dvParallel_ = true;
            MPI_Type_contiguous(3, MPI_DOUBLE, &vecType_);
            MPI_Type_contiguous(9, MPI_DOUBLE, &tensorType_);
            MPI_Type_commit(&vecType_);
            MPI_Type_commit(&tensorType_);
            MPI_Op_create((MPI_User_function *)vectorSum, 1, &opSumVec_);
            MPI_Op_create((MPI_User_function *)tensorSum, 1, &opSumTensor_);
            spilt_comm(vel_comm);
        }
    }
}

//velocity parallel only has one communicator(same color,rank_%pd=0)
void fieldMPIreducer::spilt_comm(MPI_Comm &vel_comm)
{
    int color = rank_ % pd;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank_, &vel_comm);
    MPI_Comm_rank(vel_comm, &col_rank);
    MPI_Comm_size(vel_comm, &col_size);

    // printf("WORLD RANK/SIZE: %d/%d --- ROW RANK/SIZE: %d/%d\n", rank_, nproc_, col_rank, col_size);
}

fieldMPIreducer::~fieldMPIreducer()
{
    if (pd < nproc_)
        MPI_Comm_free(&vel_comm);
}

void fieldMPIreducer::reduceField(volScalarField &vsf)
{
    List<scalar> vsfPart(vsf);
    MPI_Allreduce(
        vsfPart.data(),
        vsf.data(),
        vsf.size(),
        MPI_DOUBLE,
        MPI_SUM,
        vel_comm);
}

void fieldMPIreducer::reduceField(surfaceScalarField &ssf)
{
    List<scalar> ssfPart(ssf);
    MPI_Allreduce(
        ssfPart.data(),
        ssf.data(),
        ssf.size(),
        MPI_DOUBLE,
        MPI_SUM,
        vel_comm);
    forAll(ssf.boundaryField(), patchi) {
     if(ssf.boundaryField()[patchi].type()!="empty") {
        //Info<<ssf.boundaryField()[patchi];
        reduceField(ssf.BOUNDARY_FIELD_REF[patchi]);
      }
   }
}

void fieldMPIreducer::reduceField(volVectorField &vvf)
{
    List<vector> vvfPart(vvf);
    //Info<<vvf.boundaryField();
    MPI_Allreduce(
        vvfPart.data(),
        vvf.data(),
        vvf.size(),
        vecType_,
        opSumVec_,
        vel_comm);
    //forAll(vvf.boundaryField(), patchi)
        //reduceField(vvf.BOUNDARY_FIELD_REF[patchi]);
}

void fieldMPIreducer::reduceField(surfaceVectorField &svf)
{
    List<vector> svfPart(svf);
    //Info<<svf.boundaryField();
    MPI_Allreduce(
        svfPart.data(),
        svf.data(),
        svf.size(),
        vecType_,
        opSumVec_,
        vel_comm);
    forAll(svf.boundaryField(), patchi) {
       if(svf.boundaryField()[patchi].type()!="empty")
         reduceField(svf.BOUNDARY_FIELD_REF[patchi]);
     }
}

void fieldMPIreducer::reduceField(scalarField &sf)
{
    List<scalar> sfPart(sf);
    MPI_Allreduce(
        sfPart.data(),
        sf.data(),
        sf.size(),
        MPI_DOUBLE,
        MPI_SUM,
        vel_comm);
}

void fieldMPIreducer::reduceField(vectorField &vf)
{
    List<vector> vfPart(vf);
    MPI_Allreduce(
        vfPart.data(),
        vf.data(),
        vf.size(),
        vecType_,
        opSumVec_,
        vel_comm);
}

void fieldMPIreducer::reduceField(tensorField &vf)
{
    List<tensor> vfPart(vf);
    MPI_Allreduce(
        vfPart.data(),
        vf.data(),
        vf.size(),
        tensorType_,
        opSumTensor_,
        vel_comm);
}

void fieldMPIreducer::vectorSum(vector *in, vector *inout, int *len, MPI_Datatype *dptr)
{
    int i;
    for (i = 0; i < *len; ++i)
    {
        *inout = *in + *inout;
        in++;
        inout++;
    }
}

void fieldMPIreducer::tensorSum(tensor *in, tensor *inout, int *len, MPI_Datatype *dptr)
{
    int i;
    for (i = 0; i < *len; ++i)
    {
        *inout = *in + *inout;
        in++;
        inout++;
    }
}
