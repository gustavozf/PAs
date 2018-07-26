/*
 * Copyright (c) 2014 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Anthony Gutierrez
 */

/* @file
 * Implementation of a bi-mode branch predictor
 */

#include "cpu/pred/PAs.hh"

#include "base/bitfield.hh"
#include "base/intmath.hh"

PAsBP::PAsBP(const PAsBPParams *params)
    : BPredUnit(params),
      thread(params->numThreads),
      k(params->k),
      a(params->a),
      m(params->m),
      bitsSPHT(params->tamPred),
      bitsPBHT(params->tamHistBHT)
    {
    printf("\nINICIANDO PARAMETROS\n");
    tamPBHT = pow(2,a);
    numColSPHT = pow(2, m);
    numLinSPHT = pow(2, k);
    maskHist = mask(bitsPBHT);

    PBHT.resize(thread);
    for(int j = 0; j < thread; j++){
        PBHT[j].resize(tamPBHT);
        for(int i=0; i < tamPBHT; i++){
            PBHT[j][i] = 0;
        }
    }

    SPHT.resize(numLinSPHT);
    for(int i = 0; i < numLinSPHT; i++){
        SPHT[i].resize(numColSPHT);
        for(int j=0; j < numColSPHT; j++){
            SPHT[i][j].setBits(bitsSPHT);
        }
    }
    printf("\nPARAMETROS CRIADOS\n");
}

/*
 * For an unconditional branch we set its history such that
 * everything is set to taken. I.e., its choice predictor
 * chooses the taken array and the taken array predicts taken.
 */
void
PAsBP::uncondBranch(ThreadID tid, Addr pc, void * &bpHistory)
{
    printf("Entrou uncondBranch\n");

    unsigned powA = pow(2, a) -1;
    unsigned powK = pow(2, k)-1;
    unsigned powM = pow(2, m)-1;

    unsigned index = (pc & powA);
    unsigned linhaSPHT = (PBHT[tid][index] & powK);
    unsigned colunaSPHT = (pc & powM);

    PBHT[tid][index] = (PBHT[tid][index] << 1);

    PBHT[tid][index] |=  1;
    SPHT[linhaSPHT][colunaSPHT].increment();
    printf("Saiu uncondBranch\n\n");
}

// Apaga o historico
void
PAsBP::squash(ThreadID tid, void *bpHistory)
{
    printf("Entrou squash\n");
    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    PBHT[tid][history->index] = history->HistPBHT;

    delete history;

    printf("Saiu squash\n\n");
}

bool
PAsBP::lookup(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    printf("Entrou lookup\n");

    unsigned powA = pow(2, a) -1;
    unsigned powK = pow(2, k)-1;
    unsigned powM = pow(2, m)-1;

    BPHistory *history = new BPHistory;
    unsigned indexPBHT = (branchAddr & powA);
    unsigned linhaSPHT = (PBHT[tid][indexPBHT] & powK);
    unsigned colunaSPHT = (branchAddr & powM);
    
    bool finalPrediction = (SPHT[linhaSPHT][colunaSPHT].read() > 1);
    history->index = indexPBHT;
    history->HistPBHT = PBHT[tid][indexPBHT];

    bpHistory = static_cast<void *>(history);
    printf("Saiu lookup\n\n");
    return finalPrediction;
}

// So no primeiro nivel
void
PAsBP::btbUpdate(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    printf("Entrou btbUpdate\n");
    unsigned powA = pow(2, a) -1;
    //BPHistory *history = static_cast<BPHistory*>(bpHistory);
    PBHT[tid][(branchAddr & powA)] &= (maskHist & ~ULL(1)); //transorma o ultimo bit em 0

    //delete history;
    printf("Saiu btbUpdate\n\n");
}

void
PAsBP::update(ThreadID tid, Addr branchAddr, bool taken, void *bpHistory,
                 bool squashed)
{
    // We do not update the counters speculatively on a squash.
    // We just restore the global history register.
    
    printf("Entrou update\n");

    unsigned powA = pow(2, a) -1;
    unsigned powK = pow(2, k)-1;
    unsigned powM = pow(2, m)-1;

    unsigned index = (branchAddr & powA);
    unsigned linhaSPHT = (PBHT[tid][index] & powK);
    unsigned colunaSPHT = (branchAddr & powM);

    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    if (squashed) {
        PBHT[tid][index] = (history->HistPBHT << 1) | taken;
        printf("Saiu update (squashed)\n\n");
        return;
    }

    
    PBHT[tid][index] = (PBHT[tid][index] << 1);
    printf("Leu da PBHT\n");

    if (taken){
        printf("taken!\n");
        PBHT[tid][index] |=  1;
        printf("Leu PBHT!\n");
        SPHT[linhaSPHT][colunaSPHT].increment();
        printf("Incrementou SPHT\n");
    } else{
        printf("Not taken!\n");
        SPHT[linhaSPHT][colunaSPHT].decrement();
        printf("decrementou SPHT\n");
    }

    printf("Saiu update\n\n");
}

PAsBP*
PAsBPParams::create()
{
    return new PAsBP(this);
}
