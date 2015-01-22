/*
 * Copyright (C) 2014  mingspy@163.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
/*
* implementation of ResGuard.
* See <POSIX Muti-threads programming>
* by xiuleili 2013-7-5
*/

#pragma once

#if _MSC_VER > 1000
#include <windows.h>
///////////////////////////////////////////////////////////////////////////////

// Instances of this class will be accessed by multiple threads. So,
// all members of this class (except the constructor and destructor)
// must be thread-safe.
class ResGuard
{
public:
    ResGuard()
    {
        m_guardcount = 0;
        InitializeCriticalSection(&m_cs);
    }
    ~ResGuard()
    {
        DeleteCriticalSection(&m_cs);
    }

    // IsGuarded is used for debugging
    bool isgurded() const
    {
        return(m_guardcount > 0);
    }

public:
    class Lock
    {
    public:
        Lock(ResGuard& rg) : m_rg(rg)
        {
            m_rg.Guard();
        };
        ~Lock()
        {
            m_rg.Unguard();
        }

    private:
        ResGuard& m_rg;
    };

private:
    void Guard()
    {
        EnterCriticalSection(&m_cs);
        m_guardcount++;
    }
    void Unguard()
    {
        m_guardcount--;
        LeaveCriticalSection(&m_cs);
    }

    // Guard/Unguard can only be accessed by the nested CGuard class.
    friend class ResGuard::Lock;

private:
    CRITICAL_SECTION m_cs;
    long m_guardcount;   // # of EnterCriticalSection calls
};
#else

#include <pthread.h>


// Instances of this class will be accessed by multiple threads. So,
// all members of this class (except the constructor and destructor)
// must be thread-safe.
class ResGuard
{
public:
    ResGuard()
    {
        m_guardcount = 0;
        pthread_mutex_init(&m_cs, NULL);
    }
    ~ResGuard()
    {
        pthread_mutex_destroy(&m_cs);
    }

    // IsGuarded is used for debugging
    bool isgurded() const
    {
        return(m_guardcount > 0);
    }

public:
    class Lock
    {
    public:
        Lock(ResGuard& rg) : m_rg(rg)
        {
            m_rg.Guard();
        };
        ~Lock()
        {
            m_rg.Unguard();
        }

    private:
        ResGuard& m_rg;
    };

private:
    void Guard()
    {
        pthread_mutex_lock(&m_cs);
        m_guardcount++;
    }
    void Unguard()
    {
        m_guardcount--;
        pthread_mutex_unlock(&m_cs);
    }

    // Guard/Unguard can only be accessed by the nested Guard class.
    friend class ResGuard::Lock;

private:
    pthread_mutex_t m_cs;
    long m_guardcount;   // # of EnterCriticalSection calls
};
#endif
