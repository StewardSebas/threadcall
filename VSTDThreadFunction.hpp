/* ***************************************************************************
 * @file		VSTDThreadFunction.hpp
 * @brief		スレッドベース定義用 Class
 * @author	Sebastian
 * @date		2009/7/16
 * @version	1.0
 *  
 * @par 更新履歴：
 * - 2009/07/16	Sebastian 新規作成
 * ***************************************************************************/
#ifndef VSTDTHREADFUNCTION_H_
#define VSTDTHREADFUNCTION_H_
/* ***************************************************************************
 * including library
 * ***************************************************************************/
#include "VSTDMutex.hpp"

namespace VSTD
{
	void Sleep( unsigned int mseconds);
	/**
	 * @brief		スレッドファンクションの実行状況を定義している列挙体
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	typedef enum
	{
		/** @brief 登録されているファンクションが存在していません。 */
		THFUNC_STATE_NOTSUBMITTED ,
		/** @brief 実行待ち待機中です */
		THFUNC_STATE_WAITING,
		/** @brief 実行中です */
		THFUNC_STATE_PROCESSED,
		/** @brief 実行完了 */
		THFUNC_STATE_COMLETED
	} functionstatus_t;
	/**
	 * @brief		ThreadFunction
	 * @note		ThreadFunctionはVSTDThreadCallと連携して使用されるベースクラスです。
	 * 				ThreadFunctionクラスを継承したクラスを作成し
	 * 				仮想関数Functionをラッピングする事で実態を持ったFunctionに対して
	 * 				スレッドの呼び出し先を作成する事で使用します。
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	class ThreadFunction
	{
	private:
		/** @brief スレッドファンクションのステータスを示します。  */
		functionstatus_t	functionstate;
		/** @brief スレッドファンクションのスレッドIDを保持します。  */
		pthread_t			FunctionId;
	public:
		/** @brief スレッドファンクション用のミューテックス管理オブジェクト */
		Mutex mutex;
		/**
		 * @brief		setStatus
		 * 				ステータスの設定
		 * @note		現在のスレッドファンクションの実行状況を設定します。
		 * @param[in]	state : ステータスを指定します。
		 * @author	Sebastian
		 * @date		2009/7/16
		 */
		void setStatus(functionstatus_t state)
		{
			mutex.lock();
			functionstate=state;
			mutex.unlock();
		}
		/**
		 * @brief		getStatus
		 * 				ステータスの取得
		 * @note		現在のスレッドファンクションの実行状況を取得します。
		 * @return		ステータスを返却します。
		 * @author		Sebastian
		 * @date		2009/7/16
		 */
		functionstatus_t getStatus()
		{
			functionstatus_t state ;
			mutex.lock();
			state = functionstate;
			mutex.unlock();
			return state;
		}
		/**
		 * @brief		setThreadId
		 * 				スレッドIDの設定
		 * @note		スレッドファンクションのスレッドIDを指定します。
		 * @param[in]	threadid : スレッドIDを参照渡しします。
		 * @author	Sebastian
		 * @date		2009/7/16
		 */
		void setThreadId(pthread_t * threadid)
		{
			memcpy(&FunctionId , threadid , sizeof(pthread_t));
		}
		/**
		 * @brief		getThreadId
		 * 				スレッドIDの取得
		 * @note		スレッドファンクションのスレッドIDを取得します。
		 * @param[in]	threadid	スレッドIDを参照渡しします。
		 * @author	Sebastian
		 * @date		2009/7/16
		 */
		void getThreadId(pthread_t *threadId)
		{
			memcpy(threadId , &FunctionId , sizeof(pthread_t));
		}
		/**
		 * @brief		wait
		 * 				スレッドの一時停止
		 * @note		一定時間処理を停止します。
		 * 				実行された場合、指定したタイムアウトを迎えるか
		 * 				100milli秒経過するまでスレッドファンクションを停止します。
		 * 				ただし、ステータスが既に完了(THFUNC_STATE_COMLETED)に
		 * 				ある場合は、停止せずにそのままメソッドを完了します。
		 * @param[in]	timeout : タイムアウト時間を秒指定します。
		 * @return	待機状況をboolにて返却します。
		 * @retval	true : スレッド完了した場合
		 * @retval	false : タイムアウト、もしくは一時停止が発生した場合。
		 * @author	Sebastian
		 * @date		2009/7/16
		 */
		bool wait(int timeout)
		{
			/* *******************************************************************
			 * スレッドファンクションが既に完了済みの場合はそのまま終了します。
			 * *******************************************************************/
			if (getStatus() == THFUNC_STATE_COMLETED)
			{
				return true;
			}

			/* *******************************************************************
			 * 未完了でありタイムアウトしていない状況の場合は一定時間待機します。
			 * *******************************************************************/
			/* MilliSecに変換 */
			timeout = timeout * 1000;
			if( getStatus() != THFUNC_STATE_COMLETED
			 && timeout > 0)
			{
				/* 100milli秒停止 */
				Sleep(100);
				/* タイムアウトから100milli秒引く */
				timeout = timeout - 100;
			}
			return false;
		}
		/**
		 * @brief		ThreadFunctionのコンストラクタ
		 * @author	Sebastian
		 * @date		2009/7/16
		 */
		ThreadFunction()
		{
			/* ステータスの初期化 */
			functionstate = THFUNC_STATE_NOTSUBMITTED;
			/* スレッドIDの初期化 */
			memset(&FunctionId , 0 , sizeof(pthread_t));
		}
		/**
		 * @brief		ThreadFunctionのデストラクタ
		 * @author	Sebastian
		 * @date		2009/7/16
		 */
		virtual ~ThreadFunction()
		{
		}
		/**
		 * @brief		スレッドラッピング用仮想メソッド
		 * @author	Sebastian
		 * @date		2009/7/16
		 */
		virtual bool Function() = 0;
	};
}
#endif /*VSTDTHREADFUNCTION_H_*/
