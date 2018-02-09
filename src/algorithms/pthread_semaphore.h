
#pragma once

#include <cassert>
#include <cstdint>

#include <semaphore.h>

#include <time.h>
#include <errno.h>


namespace threading {

class semaphore
{
public:
    semaphore( const semaphore& other ) = delete;
    semaphore& operator=( const semaphore& other ) = delete;

    semaphore( int initialCount = 0 )
    {
        assert( initialCount >= 0 );
        sem_init( &m_sema, 0, initialCount );
    }

    ~semaphore()
    {
        sem_destroy( &m_sema );
    }

    void	        lock() noexcept { down(); }
    void	        unlock( int count = 1 ) noexcept { up( count ); }
    bool            empty() const noexcept { return false; }

    bool try_lock() noexcept
    {
        int rc;
        do {
            rc = sem_trywait( &m_sema );
        } while( rc == -1 && errno == EINTR );
        return !(rc == -1 && errno == EAGAIN);
    }

    bool timed_wait( uint64_t usecs ) noexcept
    {
        struct timespec ts;
        const int usecs_in_1_sec = 1000000;
        const int nsecs_in_1_sec = 1000000000;
        clock_gettime( CLOCK_REALTIME, &ts );
        ts.tv_sec += usecs / usecs_in_1_sec;
        ts.tv_nsec += (usecs % usecs_in_1_sec) * 1000;
        // sem_timedwait bombs if you have more than 1e9 in tv_nsec
        // so we have to clean things up before passing it in
        if( ts.tv_nsec > nsecs_in_1_sec ) {
            ts.tv_nsec -= nsecs_in_1_sec;
            ++ts.tv_sec;
        }

        int rc;
        do {
            rc = sem_timedwait( &m_sema, &ts );
        } while( rc == -1 && errno == EINTR );
        return !(rc == -1 && errno == ETIMEDOUT);
    }

    void down() noexcept
    {
        // http://stackoverflow.com/questions/2013181/gdb-causes-sem-wait-to-fail-with-eintr-error
        int rc;
        do {
            rc = sem_wait( &m_sema );
        } while( rc == -1 && errno == EINTR );
    }


    void up() noexcept
    {
        sem_post( &m_sema );
    }

    void up( int count ) noexcept
    {
        while( count-- > 0 )
        {
            sem_post( &m_sema );
        }
    }

    void            reset() noexcept
    {
        while( try_lock() )
        {
            ;
        }
    }
private:
    sem_t   m_sema;
};
}
