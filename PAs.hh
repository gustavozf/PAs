#include <vector>

#include "base/types.hh"
#include "cpu/pred/bpred_unit.hh"
#include "cpu/pred/sat_counter.hh"
#include "params/PAsBP.hh"

class PAsBP : public BPredUnit
{
    public:
        PasBP(const PAsBPParams *params);

        void uncondBranch(ThreadID tid, Addr pc, void * &bp_history);
        void squash(ThreadID tid, void *bp_history);
        bool lookup(ThreadID tid, Addr branch_addr, void * &bp_history);
        void btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history);
        void update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
                bool squashed);
        unsigned getGHR(ThreadID tid, void *bp_history) const;

    private:
        struct BPHistory {
            unsigned globalHistoryReg;
            // was the taken array's prediction used?
            // true: takenPred used
            // false: notPred used
            bool takenUsed;
            // prediction of the taken array
            // true: predict taken
            // false: predict not-taken
            bool takenPred;
            // prediction of the not-taken array
            // true: predict taken
            // false: predict not-taken
            bool notTakenPred;
            // the final taken/not-taken prediction
            // true: predict taken
            // false: predict not-taken
            bool finalPred;
        };

        // Nivel 1 / TAM = 2^a
        // Idexa usando "a" bits menos significativos de um addr 'B'
        // Guarda o historico de saltos, que sao varios bits
        std::vector<SatCounter> PBHT;
        // Nivel 2 / 2^k e 2^m
        // escolher linha = 'k' bits do historico de salto armazenado em PBHT
        // escolher coluna = 'm' bits menos significativos de um msm addr 'B'
        std::vector<vector<SatCounter>> SPHT;

        unsigned k; //
        unsigned a; //
        unsigned m; //
        unsigned bitsSPHT; //
        unsigned bitsPBHT; //
        unsigned tamPBHT; // 
        unsigned numColSPHT; // 2^m
};
