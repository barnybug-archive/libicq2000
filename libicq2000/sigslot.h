// sigslot.h: Signal/Slot classes
// 
// Written by Sarah Thompson (sarah@telergy.com) 2002.
// Additions by Barnaby Gray (barnaby@pickle.me.uk) 2002.
//
// License: Public domain. You are free to use this code however you like, with the proviso that
//          the author takes on no responsibility or liability for any use.
//
// This file is generated! DO NOT EDIT!
// Edit the original sigslot.pl script.
//
// QUICK DOCUMENTATION 
//		
//				(see also the full documentation at http://sigslot.sourceforge.net/)
//
//		#define switches
//			SIGSLOT_PURE_ISO			- Define this to force ISO C++ compliance. This also disables
//										  all of the thread safety support on platforms where it is 
//										  available.
//
//			SIGSLOT_USE_POSIX_THREADS	- Force use of Posix threads when using a C++ compiler other than
//										  gcc on a platform that supports Posix threads. (When using gcc,
//										  this is the default - use SIGSLOT_PURE_ISO to disable this if 
//										  necessary)
//
//			SIGSLOT_DEFAULT_MT_POLICY	- Where thread support is enabled, this defaults to multi_threaded_global.
//										  Otherwise, the default is single_threaded. #define this yourself to
//										  override the default. In pure ISO mode, anything other than
//										  single_threaded will cause a compiler error.
//
//		PLATFORM NOTES
//
//			Win32						- On Win32, the WIN32 symbol must be #defined. Most mainstream
//										  compilers do this by default, but you may need to define it
//										  yourself if your build environment is less standard. This causes
//										  the Win32 thread support to be compiled in and used automatically.
//
//			Unix/Linux/BSD, etc.		- If you're using gcc, it is assumed that you have Posix threads
//										  available, so they are used automatically. You can override this
//										  (as under Windows) with the SIGSLOT_PURE_ISO switch. If you're using
//										  something other than gcc but still want to use Posix threads, you
//										  need to #define SIGSLOT_USE_POSIX_THREADS.
//
//			ISO C++						- If none of the supported platforms are detected, or if
//										  SIGSLOT_PURE_ISO is defined, all multithreading support is turned off,
//										  along with any code that might cause a pure ISO C++ environment to
//										  complain. Before you ask, gcc -ansi -pedantic won't compile this 
//										  library, but gcc -ansi is fine. Pedantic mode seems to throw a lot of
//										  errors that aren't really there. If you feel like investigating this,
//										  please contact the author.
//
//		
//		THREADING MODES
//
//			single_threaded				- Your program is assumed to be single threaded from the point of view
//										  of signal/slot usage (i.e. all objects using signals and slots are
//										  created and destroyed from a single thread). Behaviour if objects are
//										  destroyed concurrently is undefined (i.e. you'll get the occasional
//										  segmentation fault/memory exception).
//
//			multi_threaded_global		- Your program is assumed to be multi threaded. Objects using signals and
//										  slots can be safely created and destroyed from any thread, even when
//										  connections exist. In multi_threaded_global mode, this is achieved by a
//										  single global mutex (actually a critical section on Windows because they
//										  are faster). This option uses less OS resources, but results in more
//										  opportunities for contention, possibly resulting in more context switches
//										  than are strictly necessary.
//
//			multi_threaded_local		- Behaviour in this mode is essentially the same as multi_threaded_global,
//										  except that each signal, and each object that inherits has_slots, all 
//										  have their own mutex/critical section. In practice, this means that
//										  mutex collisions (and hence context switches) only happen if they are
//										  absolutely essential. However, on some platforms, creating a lot of 
//										  mutexes can slow down the whole OS, so use this option with care.
//
//		USING THE LIBRARY
//
//			See the full documentation at http://sigslot.sourceforge.net/
//
//

#ifndef SIGSLOT_H__
#define SIGSLOT_H__

#include <set>
#include <list>

#if defined(SIGSLOT_PURE_ISO) || (!defined(WIN32) && !defined(__GNUG__) && !defined(SIGSLOT_USE_POSIX_THREADS))
#	define _SIGSLOT_SINGLE_THREADED
#elif defined(WIN32)
#	define _SIGSLOT_HAS_WIN32_THREADS
#	include <windows.h>
#elif defined(__GNUG__) || defined(SIGSLOT_USE_POSIX_THREADS)
#	define _SIGSLOT_HAS_POSIX_THREADS
#	include <pthread.h>
#else
#	define _SIGSLOT_SINGLE_THREADED
#endif

#ifndef SIGSLOT_DEFAULT_MT_POLICY
#	ifdef _SIGSLOT_SINGLE_THREADED
#		define SIGSLOT_DEFAULT_MT_POLICY single_threaded
#	else
#		define SIGSLOT_DEFAULT_MT_POLICY multi_threaded_local
#	endif
#endif


namespace sigslot {

	class single_threaded
	{
	public:
		single_threaded()
		{
			;
		}

		virtual ~single_threaded()
		{
			;
		}

		virtual void lock()
		{
			;
		}

		virtual void unlock()
		{
			;
		}
	};

#ifdef _SIGSLOT_HAS_WIN32_THREADS
	// The multi threading policies only get compiled in if they are enabled.
	class multi_threaded_global
	{
	public:
		multi_threaded_global()
		{
			static bool isinitialised = false;

			if(!isinitialised)
			{
				InitializeCriticalSection(get_critsec());
				isinitialised = true;
			}
		}

		multi_threaded_global(const multi_threaded_global&)
		{
			;
		}

		virtual ~multi_threaded_global()
		{
			;
		}

		virtual void lock()
		{
			EnterCriticalSection(get_critsec());
		}

		virtual void unlock()
		{
			LeaveCriticalSection(get_critsec());
		}

	private:
		CRITICAL_SECTION* get_critsec()
		{
			static CRITICAL_SECTION g_critsec;
			return &g_critsec;
		}
	};

	class multi_threaded_local
	{
	public:
		multi_threaded_local()
		{
			InitializeCriticalSection(&m_critsec);
		}

		multi_threaded_local(const multi_threaded_local&)
		{
			InitializeCriticalSection(&m_critsec);
		}

		virtual ~multi_threaded_local()
		{
			DeleteCriticalSection(&m_critsec);
		}

		virtual void lock()
		{
			EnterCriticalSection(&m_critsec);
		}

		virtual void unlock()
		{
			LeaveCriticalSection(&m_critsec);
		}

	private:
		CRITICAL_SECTION m_critsec;
	};
#endif // _SIGSLOT_HAS_WIN32_THREADS

#ifdef _SIGSLOT_HAS_POSIX_THREADS
	// The multi threading policies only get compiled in if they are enabled.
	class multi_threaded_global
	{
	public:
		multi_threaded_global()
		{
			pthread_mutex_init(get_mutex(), NULL);
		}

		multi_threaded_global(const multi_threaded_global&)
		{
			;
		}

		virtual ~multi_threaded_global()
		{
			;
		}

		virtual void lock()
		{
			pthread_mutex_lock(get_mutex());
		}

		virtual void unlock()
		{
			pthread_mutex_unlock(get_mutex());
		}

	private:
		pthread_mutex_t* get_mutex()
		{
			static pthread_mutex_t g_mutex;
			return &g_mutex;
		}
	};

	class multi_threaded_local
	{
	public:
		multi_threaded_local()
		{
			pthread_mutex_init(&m_mutex, NULL);
		}

		multi_threaded_local(const multi_threaded_local&)
		{
			pthread_mutex_init(&m_mutex, NULL);
		}

		virtual ~multi_threaded_local()
		{
			pthread_mutex_destroy(&m_mutex);
		}

		virtual void lock()
		{
			pthread_mutex_lock(&m_mutex);
		}

		virtual void unlock()
		{
			pthread_mutex_unlock(&m_mutex);
		}

	private:
		pthread_mutex_t m_mutex;
	};
#endif // _SIGSLOT_HAS_POSIX_THREADS

	template<class mt_policy>
	class lock_block
	{
	public:
		mt_policy *m_mutex;

		lock_block(mt_policy *mtx)
			: m_mutex(mtx)
		{
			m_mutex->lock();
		}

		~lock_block()
		{
			m_mutex->unlock();
		}
	};

	template<class mt_policy>
	class has_slots;

	template<class mt_policy>
	class _connection_base0
	{
	public:
		virtual has_slots<mt_policy>* getdest() const = 0;
		virtual void emit() = 0;
		virtual _connection_base0<mt_policy>* clone() = 0;
		virtual _connection_base0<mt_policy>* duplicate(has_slots<mt_policy>* pnewdest) = 0;
	};

	template<class arg1_type, class mt_policy>
	class _connection_base1
	{
	public:
		virtual has_slots<mt_policy>* getdest() const = 0;
		virtual void emit(arg1_type) = 0;
		virtual _connection_base1<arg1_type, mt_policy>* clone() = 0;
		virtual _connection_base1<arg1_type, mt_policy>* duplicate(has_slots<mt_policy>* pnewdest) = 0;
	};

	template<class arg1_type, class arg2_type, class mt_policy>
	class _connection_base2
	{
	public:
		virtual has_slots<mt_policy>* getdest() const = 0;
		virtual void emit(arg1_type, arg2_type) = 0;
		virtual _connection_base2<arg1_type, arg2_type, mt_policy>* clone() = 0;
		virtual _connection_base2<arg1_type, arg2_type, mt_policy>* duplicate(has_slots<mt_policy>* pnewdest) = 0;
	};

	template<class mt_policy>
	class _signal_base : public has_slots<mt_policy>
	{
	public:
		virtual void slot_disconnect(has_slots<mt_policy>* pslot) = 0;
		virtual void slot_duplicate(const has_slots<mt_policy>* poldslot, has_slots<mt_policy>* pnewslot) = 0;
	};

	template<class mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
	class has_slots : public mt_policy 
	{
	private:
		typedef std::set<_signal_base<mt_policy> *> sender_set;
		typedef sender_set::const_iterator const_iterator;

	public:
		has_slots()
		{
			;
		}

		has_slots(const has_slots& hs)
			: mt_policy(hs)
		{
			lock_block<mt_policy> lock(this);
			const_iterator it = hs.m_senders.begin();
			const_iterator itEnd = hs.m_senders.end();

			while(it != itEnd)
			{
				(*it)->slot_duplicate(&hs, this);
				m_senders.insert(*it);
				++it;
			}
		} 

		void signal_connect(_signal_base<mt_policy>* sender)
		{
			lock_block<mt_policy> lock(this);
			m_senders.insert(sender);
		}

		void signal_disconnect(_signal_base<mt_policy>* sender)
		{
			lock_block<mt_policy> lock(this);
			m_senders.erase(sender);
		}

		virtual ~has_slots()
		{
			disconnect_all();
		}

		void disconnect_all()
		{
			lock_block<mt_policy> lock(this);
			const_iterator it = m_senders.begin();
			const_iterator itEnd = m_senders.end();

			while(it != itEnd)
			{
				(*it)->slot_disconnect(this);
				++it;
			}

			m_senders.erase(m_senders.begin(), m_senders.end());
		}

	private:
		sender_set m_senders;
	};

	template<class mt_policy>
	class _signal_base0 : public _signal_base<mt_policy>
	{
	public:
		typedef std::list<_connection_base0<mt_policy> *>
			connections_list;

		_signal_base0()
		{
			;
		}

		_signal_base0(const _signal_base0<mt_policy>& s)
			: _signal_base<mt_policy>(s)
		{
			lock_block<mt_policy> lock(this);
			connections_list::const_iterator it = s.m_connected_slots.begin();
			connections_list::const_iterator itEnd = s.m_connected_slots.end();

			while(it != itEnd)
			{
				(*it)->getdest()->signal_connect(this);
				m_connected_slots.push_back((*it)->clone());

				++it;
			}
		}

		void slot_duplicate(const has_slots<mt_policy>* oldtarget, has_slots<mt_policy>* newtarget)
		{
			lock_block<mt_policy> lock(this);
			connections_list::iterator it = m_connected_slots.begin();
			connections_list::iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				if((*it)->getdest() == oldtarget)
				{
					m_connected_slots.push_back((*it)->duplicate(newtarget));
				}

				++it;
			}
		}

		~_signal_base0()
		{
			disconnect_all();
		}

		void disconnect_all()
		{
			lock_block<mt_policy> lock(this);
			connections_list::const_iterator it = m_connected_slots.begin();
			connections_list::const_iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				(*it)->getdest()->signal_disconnect(this);
				delete *it;

				++it;
			}

			m_connected_slots.erase(m_connected_slots.begin(), m_connected_slots.end());
		}

		void disconnect(has_slots<mt_policy>* pclass)
		{
			lock_block<mt_policy> lock(this);
			connections_list::iterator it = m_connected_slots.begin();
			connections_list::iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				if((*it)->getdest() == pclass)
				{
					delete *it;
					m_connected_slots.erase(it);
					pclass->signal_disconnect(this);
					return;
				}

				++it;
			}
		}

		void slot_disconnect(has_slots<mt_policy>* pslot)
		{
			lock_block<mt_policy> lock(this);
			connections_list::iterator it = m_connected_slots.begin();
			connections_list::iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				connections_list::iterator itNext = it;
				++itNext;

				if((*it)->getdest() == pslot)
				{
					m_connected_slots.erase(it);
					delete *it;
				}

				it = itNext;
			}
		}

	protected:
		connections_list m_connected_slots;   
	};

	template<class arg1_type, class mt_policy>
	class _signal_base1 : public _signal_base<mt_policy>
	{
	public:
		typedef std::list<_connection_base1<arg1_type, mt_policy> *>
			connections_list;

		_signal_base1()
		{
			;
		}

		_signal_base1(const _signal_base1<arg1_type, mt_policy>& s)
			: _signal_base<mt_policy>(s)
		{
			lock_block<mt_policy> lock(this);
			connections_list::const_iterator it = s.m_connected_slots.begin();
			connections_list::const_iterator itEnd = s.m_connected_slots.end();

			while(it != itEnd)
			{
				(*it)->getdest()->signal_connect(this);
				m_connected_slots.push_back((*it)->clone());

				++it;
			}
		}

		void slot_duplicate(const has_slots<mt_policy>* oldtarget, has_slots<mt_policy>* newtarget)
		{
			lock_block<mt_policy> lock(this);
			connections_list::iterator it = m_connected_slots.begin();
			connections_list::iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				if((*it)->getdest() == oldtarget)
				{
					m_connected_slots.push_back((*it)->duplicate(newtarget));
				}

				++it;
			}
		}

		~_signal_base1()
		{
			disconnect_all();
		}

		void disconnect_all()
		{
			lock_block<mt_policy> lock(this);
			connections_list::const_iterator it = m_connected_slots.begin();
			connections_list::const_iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				(*it)->getdest()->signal_disconnect(this);
				delete *it;

				++it;
			}

			m_connected_slots.erase(m_connected_slots.begin(), m_connected_slots.end());
		}

		void disconnect(has_slots<mt_policy>* pclass)
		{
			lock_block<mt_policy> lock(this);
			connections_list::iterator it = m_connected_slots.begin();
			connections_list::iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				if((*it)->getdest() == pclass)
				{
					delete *it;
					m_connected_slots.erase(it);
					pclass->signal_disconnect(this);
					return;
				}

				++it;
			}
		}

		void slot_disconnect(has_slots<mt_policy>* pslot)
		{
			lock_block<mt_policy> lock(this);
			connections_list::iterator it = m_connected_slots.begin();
			connections_list::iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				connections_list::iterator itNext = it;
				++itNext;

				if((*it)->getdest() == pslot)
				{
					m_connected_slots.erase(it);
					delete *it;
				}

				it = itNext;
			}
		}

	protected:
		connections_list m_connected_slots;   
	};

	template<class arg1_type, class arg2_type, class mt_policy>
	class _signal_base2 : public _signal_base<mt_policy>
	{
	public:
		typedef std::list<_connection_base2<arg1_type, arg2_type, mt_policy> *>
			connections_list;

		_signal_base2()
		{
			;
		}

		_signal_base2(const _signal_base2<arg1_type, arg2_type, mt_policy>& s)
			: _signal_base<mt_policy>(s)
		{
			lock_block<mt_policy> lock(this);
			connections_list::const_iterator it = s.m_connected_slots.begin();
			connections_list::const_iterator itEnd = s.m_connected_slots.end();

			while(it != itEnd)
			{
				(*it)->getdest()->signal_connect(this);
				m_connected_slots.push_back((*it)->clone());

				++it;
			}
		}

		void slot_duplicate(const has_slots<mt_policy>* oldtarget, has_slots<mt_policy>* newtarget)
		{
			lock_block<mt_policy> lock(this);
			connections_list::iterator it = m_connected_slots.begin();
			connections_list::iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				if((*it)->getdest() == oldtarget)
				{
					m_connected_slots.push_back((*it)->duplicate(newtarget));
				}

				++it;
			}
		}

		~_signal_base2()
		{
			disconnect_all();
		}

		void disconnect_all()
		{
			lock_block<mt_policy> lock(this);
			connections_list::const_iterator it = m_connected_slots.begin();
			connections_list::const_iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				(*it)->getdest()->signal_disconnect(this);
				delete *it;

				++it;
			}

			m_connected_slots.erase(m_connected_slots.begin(), m_connected_slots.end());
		}

		void disconnect(has_slots<mt_policy>* pclass)
		{
			lock_block<mt_policy> lock(this);
			connections_list::iterator it = m_connected_slots.begin();
			connections_list::iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				if((*it)->getdest() == pclass)
				{
					delete *it;
					m_connected_slots.erase(it);
					pclass->signal_disconnect(this);
					return;
				}

				++it;
			}
		}

		void slot_disconnect(has_slots<mt_policy>* pslot)
		{
			lock_block<mt_policy> lock(this);
			connections_list::iterator it = m_connected_slots.begin();
			connections_list::iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				connections_list::iterator itNext = it;
				++itNext;

				if((*it)->getdest() == pslot)
				{
					m_connected_slots.erase(it);
					delete *it;
				}

				it = itNext;
			}
		}

	protected:
		connections_list m_connected_slots;   
	};

	template<class dest_type, class mt_policy>
	class _connection0 : public _connection_base0<mt_policy>
	{
	public:
		_connection0()
		{
			pobject = NULL;
			pmemfun = NULL;
		}

		_connection0(dest_type* pobject, void (dest_type::*pmemfun)())
		{
			m_pobject = pobject;
			m_pmemfun = pmemfun;
		}

		virtual _connection_base0<mt_policy>* clone()
		{
			return new _connection0<dest_type, mt_policy>(*this);
		}

		virtual _connection_base0<mt_policy>* duplicate(has_slots<mt_policy>* pnewdest)
		{
			return new _connection0<dest_type, mt_policy>((dest_type *)pnewdest, m_pmemfun);
		}

		virtual void emit()
		{
			(m_pobject->*m_pmemfun)();
		}

		virtual has_slots<mt_policy>* getdest() const
		{
			return m_pobject;
		}

	private:
		dest_type* m_pobject;
		void (dest_type::* m_pmemfun)();
	};

	template<class dest_type, class arg1_type, class mt_policy>
	class _connection1 : public _connection_base1<arg1_type, mt_policy>
	{
	public:
		_connection1()
		{
			pobject = NULL;
			pmemfun = NULL;
		}

		_connection1(dest_type* pobject, void (dest_type::*pmemfun)(arg1_type))
		{
			m_pobject = pobject;
			m_pmemfun = pmemfun;
		}

		virtual _connection_base1<arg1_type, mt_policy>* clone()
		{
			return new _connection1<dest_type, arg1_type, mt_policy>(*this);
		}

		virtual _connection_base1<arg1_type, mt_policy>* duplicate(has_slots<mt_policy>* pnewdest)
		{
			return new _connection1<dest_type, arg1_type, mt_policy>((dest_type *)pnewdest, m_pmemfun);
		}

		virtual void emit(arg1_type a1)
		{
			(m_pobject->*m_pmemfun)(a1);
		}

		virtual has_slots<mt_policy>* getdest() const
		{
			return m_pobject;
		}

	private:
		dest_type* m_pobject;
		void (dest_type::* m_pmemfun)(arg1_type);
	};

	template<class dest_type, class arg1_type, class arg2_type, class mt_policy>
	class _connection2 : public _connection_base2<arg1_type, arg2_type, mt_policy>
	{
	public:
		_connection2()
		{
			pobject = NULL;
			pmemfun = NULL;
		}

		_connection2(dest_type* pobject, void (dest_type::*pmemfun)(arg1_type, arg2_type))
		{
			m_pobject = pobject;
			m_pmemfun = pmemfun;
		}

		virtual _connection_base2<arg1_type, arg2_type, mt_policy>* clone()
		{
			return new _connection2<dest_type, arg1_type, arg2_type, mt_policy>(*this);
		}

		virtual _connection_base2<arg1_type, arg2_type, mt_policy>* duplicate(has_slots<mt_policy>* pnewdest)
		{
			return new _connection2<dest_type, arg1_type, arg2_type, mt_policy>((dest_type *)pnewdest, m_pmemfun);
		}

		virtual void emit(arg1_type a1, arg2_type a2)
		{
			(m_pobject->*m_pmemfun)(a1, a2);
		}

		virtual has_slots<mt_policy>* getdest() const
		{
			return m_pobject;
		}

	private:
		dest_type* m_pobject;
		void (dest_type::* m_pmemfun)(arg1_type, arg2_type);
	};

	template<class mt_policy>
	class _sig_connection0;
	template<class arg1_type, class mt_policy>
	class _sig_connection1;
	template<class arg1_type, class arg2_type, class mt_policy>
	class _sig_connection2;
	template<class mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
	class signal0 : public _signal_base0<mt_policy>
	{
	public:
		signal0()
		{
			;
		}

		signal0(const signal0<mt_policy>& s)
			: _signal_base0<mt_policy>(s)
		{
			;
		}

		template<class dest_type>
			void connect(dest_type* pclass, void (dest_type::*pmemfun)())
		{
			lock_block<mt_policy> lock(this);
			_connection0<dest_type, mt_policy>* conn
			    = new _connection0<dest_type, mt_policy>(pclass, pmemfun);
			m_connected_slots.push_back(conn);
			pclass->signal_connect(this);
		}

		void connect(signal0<mt_policy>& chainsig)
		{
			lock_block<mt_policy> lock(this);
			_sig_connection0<mt_policy>* conn = new _sig_connection0<mt_policy>(chainsig);
			m_connected_slots.push_back(conn);
			chainsig.signal_connect(this);
		}

		void emit()
		{
			lock_block<mt_policy> lock(this);
			connections_list::const_iterator itNext, it = m_connected_slots.begin();
			connections_list::const_iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				itNext = it;
				++itNext;

				(*it)->emit();

				it = itNext;
			}
		}

		void operator()()
		{
			lock_block<mt_policy> lock(this);
			connections_list::const_iterator itNext, it = m_connected_slots.begin();
			connections_list::const_iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				itNext = it;
				++itNext;

				(*it)->emit();

				it = itNext;
			}
		}
	};

	template<class arg1_type, class mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
	class signal1 : public _signal_base1<arg1_type, mt_policy>
	{
	public:
		signal1()
		{
			;
		}

		signal1(const signal1<arg1_type, mt_policy>& s)
			: _signal_base1<arg1_type, mt_policy>(s)
		{
			;
		}

		template<class dest_type>
			void connect(dest_type* pclass, void (dest_type::*pmemfun)(arg1_type))
		{
			lock_block<mt_policy> lock(this);
			_connection1<dest_type, arg1_type, mt_policy>* conn
			    = new _connection1<dest_type, arg1_type, mt_policy>(pclass, pmemfun);
			m_connected_slots.push_back(conn);
			pclass->signal_connect(this);
		}

		void connect(signal1<arg1_type, mt_policy>& chainsig)
		{
			lock_block<mt_policy> lock(this);
			_sig_connection1<arg1_type, mt_policy>* conn = new _sig_connection1<arg1_type, mt_policy>(chainsig);
			m_connected_slots.push_back(conn);
			chainsig.signal_connect(this);
		}

		void emit(arg1_type a1)
		{
			lock_block<mt_policy> lock(this);
			connections_list::const_iterator itNext, it = m_connected_slots.begin();
			connections_list::const_iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				itNext = it;
				++itNext;

				(*it)->emit(a1);

				it = itNext;
			}
		}

		void operator()(arg1_type a1)
		{
			lock_block<mt_policy> lock(this);
			connections_list::const_iterator itNext, it = m_connected_slots.begin();
			connections_list::const_iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				itNext = it;
				++itNext;

				(*it)->emit(a1);

				it = itNext;
			}
		}
	};

	template<class arg1_type, class arg2_type, class mt_policy = SIGSLOT_DEFAULT_MT_POLICY>
	class signal2 : public _signal_base2<arg1_type, arg2_type, mt_policy>
	{
	public:
		signal2()
		{
			;
		}

		signal2(const signal2<arg1_type, arg2_type, mt_policy>& s)
			: _signal_base2<arg1_type, arg2_type, mt_policy>(s)
		{
			;
		}

		template<class dest_type>
			void connect(dest_type* pclass, void (dest_type::*pmemfun)(arg1_type, arg2_type))
		{
			lock_block<mt_policy> lock(this);
			_connection2<dest_type, arg1_type, arg2_type, mt_policy>* conn
			    = new _connection2<dest_type, arg1_type, arg2_type, mt_policy>(pclass, pmemfun);
			m_connected_slots.push_back(conn);
			pclass->signal_connect(this);
		}

		void connect(signal2<arg1_type, arg2_type, mt_policy>& chainsig)
		{
			lock_block<mt_policy> lock(this);
			_sig_connection2<arg1_type, arg2_type, mt_policy>* conn = new _sig_connection2<arg1_type, arg2_type, mt_policy>(chainsig);
			m_connected_slots.push_back(conn);
			chainsig.signal_connect(this);
		}

		void emit(arg1_type a1, arg2_type a2)
		{
			lock_block<mt_policy> lock(this);
			connections_list::const_iterator itNext, it = m_connected_slots.begin();
			connections_list::const_iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				itNext = it;
				++itNext;

				(*it)->emit(a1, a2);

				it = itNext;
			}
		}

		void operator()(arg1_type a1, arg2_type a2)
		{
			lock_block<mt_policy> lock(this);
			connections_list::const_iterator itNext, it = m_connected_slots.begin();
			connections_list::const_iterator itEnd = m_connected_slots.end();

			while(it != itEnd)
			{
				itNext = it;
				++itNext;

				(*it)->emit(a1, a2);

				it = itNext;
			}
		}
	};

// signal chaining connection types added by Barnaby Gray 24/11/2002

	template<class mt_policy>
	class _sig_connection0 : public _connection_base0<mt_policy>
	{
	public:
		_sig_connection0(signal0<mt_policy>& sig)
		: m_signal(sig)
		{
		}

		_sig_connection0(const _sig_connection0<mt_policy>& conn)
		: m_signal( conn.m_signal )
		{
		}

		virtual _connection_base0<mt_policy>* clone()
		{
			return new _sig_connection0<mt_policy>(*this);
		}

		virtual _connection_base0<mt_policy>* duplicate(has_slots<mt_policy>* pnewdest)
		{
			return new _sig_connection0<mt_policy>(m_signal);
		}

		virtual void emit()
		{
			m_signal.emit();
		}

		virtual has_slots<mt_policy>* getdest() const
		{
			return &m_signal;
		}

	private:
		signal0<mt_policy>& m_signal;
	};

	template<class arg1_type, class mt_policy>
	class _sig_connection1 : public _connection_base1<arg1_type, mt_policy>
	{
	public:
		_sig_connection1(signal1<arg1_type, mt_policy>& sig)
		: m_signal(sig)
		{
		}

		_sig_connection1(const _sig_connection1<arg1_type, mt_policy>& conn)
		: m_signal( conn.m_signal )
		{
		}

		virtual _connection_base1<arg1_type, mt_policy>* clone()
		{
			return new _sig_connection1<arg1_type, mt_policy>(*this);
		}

		virtual _connection_base1<arg1_type, mt_policy>* duplicate(has_slots<mt_policy>* pnewdest)
		{
			return new _sig_connection1<arg1_type, mt_policy>(m_signal);
		}

		virtual void emit(arg1_type a1)
		{
			m_signal.emit(a1);
		}

		virtual has_slots<mt_policy>* getdest() const
		{
			return &m_signal;
		}

	private:
		signal1<arg1_type, mt_policy>& m_signal;
	};

	template<class arg1_type, class arg2_type, class mt_policy>
	class _sig_connection2 : public _connection_base2<arg1_type, arg2_type, mt_policy>
	{
	public:
		_sig_connection2(signal2<arg1_type, arg2_type, mt_policy>& sig)
		: m_signal(sig)
		{
		}

		_sig_connection2(const _sig_connection2<arg1_type, arg2_type, mt_policy>& conn)
		: m_signal( conn.m_signal )
		{
		}

		virtual _connection_base2<arg1_type, arg2_type, mt_policy>* clone()
		{
			return new _sig_connection2<arg1_type, arg2_type, mt_policy>(*this);
		}

		virtual _connection_base2<arg1_type, arg2_type, mt_policy>* duplicate(has_slots<mt_policy>* pnewdest)
		{
			return new _sig_connection2<arg1_type, arg2_type, mt_policy>(m_signal);
		}

		virtual void emit(arg1_type a1, arg2_type a2)
		{
			m_signal.emit(a1, a2);
		}

		virtual has_slots<mt_policy>* getdest() const
		{
			return &m_signal;
		}

	private:
		signal2<arg1_type, arg2_type, mt_policy>& m_signal;
	};

}; // namespace sigslot

#endif // SIGSLOT_H__

