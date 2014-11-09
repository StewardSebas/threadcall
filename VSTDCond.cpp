/* ***************************************************************************
 * @file		VSTDCond.cpp
 * @brief		条件変数シグナル送受信用 Class
 * @see		POSIX Thread <pthread.h>
 * @author		Sebastian
 * @date		2009/7/16
 * @version	1.0
 * @par 更新履歴：
 * - 2009/07/16	Sebastian 新規作成
 * ***************************************************************************/
#include "VSTDCond.hpp"
namespace VSTD
{
	/**
	 * @brief		VSTDCondのコンストラクタ
	 * @note		ミューテックスと条件変数の初期化
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	Condition::Condition(void)
	{
		created = true;

		pthread_mutexattr_init(&mutex_attr);
		pthread_mutex_init(&mutex_lock,&mutex_attr);
		pthread_cond_init(&signal,NULL);
	}
	/**
	 * @brief		Conditionのデストラクタ
	 * @note		ミューテックスと条件変の破棄。
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	Condition::~Condition(void)
	{
		pthread_cond_destroy(&signal);
		pthread_mutex_destroy(&mutex_lock);
	}
	/**
	 * @brief		set
	 * 				シグナル送信
	 * @note		waitメソッドにて待機中のスレッドに対してシグナルを送信し
	 * 				イベントを発生させます。
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	void Condition::set()
	{
		pthread_mutex_lock(&mutex_lock);
		pthread_mutex_unlock(&mutex_lock);
		pthread_cond_signal(&signal);
	}
	/**
	 * @brief		wait
	 * 				シグナル待機
	 * @note		スレッドをサスペンドさせシグナル受信を待ちます。
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	bool Condition::wait()
	{
		pthread_mutex_lock(&mutex_lock);
		pthread_cond_wait(&signal,&mutex_lock);
		return true;
	}
	/**
	 * @brief		reset
	 * 				シグナルの初期化
	 * @note		ミューテックスを解除する事で条件変数を初期化します。
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	void Condition::reset()
	{
		pthread_mutex_unlock(&mutex_lock);
	}
}
