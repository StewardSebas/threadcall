/* ***************************************************************************
 * @file		VSTDMutex.cpp
 * @brief		相互排他管理用 Class
 * @author	Sebastian
 * @date		2009/7/16
 * @version	1.0
 * 
 * @par 更新履歴：
 * - 2009/07/16	Sebastian 新規作成
 * ***************************************************************************/
#include "VSTDMutex.hpp"
namespace VSTD
{
	/**
	 * @brief		VSTDMutexのコンストラクタ
	 * @note		ミューテックスのIDと属性を初期化します。
	 * @author		Sebastian
	 * @date		2009/7/16
	 */
	Mutex::Mutex(void)
	{
		created = true;
		pthread_mutexattr_init(&mutex_attr);
		pthread_mutex_init(&mutex_id,&mutex_attr);
	}
	/**
	 * @brief		Mutexのデストラクタ
	 * @note		ミューテックスのIDを破棄します。
	 * @author		Sebastian
	 * @date		2009/7/16
	 */
	Mutex::~Mutex(void)
	{
		pthread_mutex_lock(&mutex_id);
		pthread_mutex_unlock(&mutex_id);
		pthread_mutex_destroy(&mutex_id);
	}
	/**
	 * @brief		lock
	 * 				ミューテックスのロック
	 * @note		スレッドをロックして排他制御を許可します。
	 * @author		Sebastian
	 * @date		2009/7/16
	 */
	void Mutex::lock()
	{
		int result;
		/* *******************************************************************
		 * ロック処理
		 * *******************************************************************/
		pthread_t id = pthread_self();
		/* ミューテックスは指定されたスレッドで既にロックされている */
		if(owner_id == id )
		{
			perror("lock:ミューテックスは既にロックされています。");
			return;
		}
		try
		{
			/* ミューテックスをロック */
			result = pthread_mutex_lock(&mutex_id);
			switch(result)
			{
			case EINVAL:
				perror("lock:ミューテックスが適切に初期化されていません。");
				return ;
			case EDEADLK:
				perror("lock:ミューテックスは既にロックされています。");
				return;
			}
			/* 現在ミューテックスを保持しているスレッドのIDを保持 */
			owner_id = id;
		}
		/* *******************************************************************
		 * 例外処理
		 * *******************************************************************/
		catch( char *descript )
		{
			throw descript;
		}
	}
	/**
	 * @brief		unlock
	 * 				ミューテックスのアンロック
	 * @note		スレッドをアンロックして排他制御を終了します。
	 * @author		Sebastian
	 * @date		2009/7/16
	 */
	void Mutex::unlock()
	{
		int result;
		/* *******************************************************************
		 * アンロック処理
		 * *******************************************************************/
		try
		{
			pthread_t id = pthread_self();
			if(id != owner_id)
			{
				throw "unlock:スレッドはミューテックスを所有していません。";
			}
			memset(&owner_id,0,sizeof(pthread_t));
			result = pthread_mutex_unlock(&mutex_id);
			switch(result)
			{
			case EINVAL:
				throw "unlock:ミューテックスが適切に初期化されていません。";
			case EPERM:
				throw "unlock:スレッドはミューテックスを所有していません。";
			}
		}
		/* *******************************************************************
		 * 例外処理
		 * *******************************************************************/
		catch( char *descript )
		{
			throw descript;
		}
	}
}
