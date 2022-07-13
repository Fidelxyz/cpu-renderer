#include "utils/omp_locker.hpp"

OmpLocker::OmpLocker(omp_lock_t *omp_lock) {
    this->omp_lock = omp_lock;
    omp_set_lock(omp_lock);
}

OmpLocker::~OmpLocker() { omp_unset_lock(omp_lock); }