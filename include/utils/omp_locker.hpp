#pragma once
#ifndef OMP_LOCKER_H
#define OMP_LOCKER_H

#include <omp.h>

class OmpLocker {
   public:
    OmpLocker(omp_lock_t *omp_lock);
    ~OmpLocker();

   private:
    omp_lock_t *omp_lock;
};

#endif