/* ***************************************************************************
 * @file		VSTDThreadCall.hpp
 * @brief		スレッドイベント呼び出し用 Class
 * @see		VSTDThreadFunction.hpp / VSTDCond.hpp
 * @author	Sebastian
 * @date		2009/7/16
 * @version	1.0
 *  
 * 
 * @par 更新履歴：
 * - 2009/07/16	Sebastian 新規作成
 * ***************************************************************************/
#ifndef THREAD_CLASS
#define THREAD_CLASS
/* ***************************************************************************
 * including library
 * ***************************************************************************/
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string>
#include "VSTDCond.hpp"
#include "VSTDThreadFunction.hpp"

namespace VSTD
{
	/** @brief 登録可能なスレッドの最大数 */
	#define MAX_THREAD 100
	/**
	 * @brief 	スレッドステータス指定用列挙体
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	typedef enum
	{
		/** @brief スレッドファンクションを実行中です。 */
		TH_STAT_BUSY		,
		/** @brief スレッドは実行待機中です。 */
		TH_STAT_WAIT		,
		/** @brief スレッドは稼働していません。 */
		TH_STAT_DOWN		,
		/** @brief スレッドは停止中です。 */
		TH_STAT_SHUTDOWN	,
		/** @brief スレッドは指定の */
		TH_STAT_FAULT
	} ThreadState_t;
	/**
	 * @brief		スレッドの実行種別設定用列挙体
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	typedef enum
	{
		/** @brief スレッドの実行種別をイベントドリブンに指定(default) */
		TH_TYP_EVENTDRIVEN,
		/** @brief スレッドの実行種別をタイマーインターバルに指定 */
		TH_TYP_INTERVAL
	} ThreadType_t;
	/**
	 *  @brief スレッド実行時のエラー状態指定用列挙体
	 */
	typedef enum
	{
		/** @brief エラー無し。正常 */
		TH_ERR_NOERROR			= 0 ,
		/** @brief ミューテックス作成中 */
		TH_ERR_MUTEX_CREATE		= 0x01 ,
		/** @brief 条件変数作成中 */
		TH_ERR_COND_CREATE		= 0x02 ,
		/** @brief スレッド作成中 */
		TH_ERR_THREAD_CREATE	= 0x04 ,
		/** @brief 不明 */
		TH_ERR_UNKNOWN			= 0x08 ,
		/** @brief 条件変数の指定に異常がある */
		TH_ERR_ILLEGAL_USE_COND	= 0x10 ,
		/** @brief メモリーエラー */
		TH_ERR_MEMORY_ERR		= 0x20
	} threaderror_t;
	/**
	 * @brief	ThreadCall
	 * @note	ThreadCallはThreadCallクラスを継承したクラスを作成し
	 * 			仮想メソッド[onFunction]をラッピングする事で
	 * 			実態を持ったFunctionに対して呼び出し先を作成する事で使用します。
	 * 			ラッピング先の派生クラスをインスタンス化の後にsetFunctionを呼出事で
	 * 			作成されたスレッドに対してシグナルが送信され、スレッドが実行されます。
	 * 			また、setFunctionの引数としてVSTDThreadFunctionの派生クラスを
	 * 			使用する事で、VSTDThreadFunctionの派生クラスにて定義された
	 * 			実行内容がスレッドとして実行されます
	 * @author	Sebastian
	 * @date	2009/7/16
	 */
	class ThreadCall
	{
		private:
			/* ***************************************************************
			 * プライベートメンバ変数
			 * ***************************************************************/
			/* @brief スレッドの条件変数操作用オブジェクト */
			Condition			condition;
			/** @brief スレッド実行フラグ */
			bool				running;
			/** @brief 実行スレッド用スレッドID */
			pthread_t			threadhandle;
			/** @brief スレッドのID */
			pthread_t			threadId;
			/** @brief スレッドのスレッド属性 */
			pthread_attr_t	thread_attr;
			/** @brief スレッドのスケジュールパラメータ */
			sched_param		thread_sched_param;
			/** @brief 条件変数の待ち行列を格納用 */
			void **			threadQueue;
			/** @brief シグナル待機中条件変数の最大数 */
			unsigned int		threadQueDepath;
			/** @brief 現在の条件変数の数 */
			unsigned int		currentThreadQueue;
			/**　@brief 現在処理されている条件変数　*/
			void *				ProcessQueue;
			/**　@brief スレッドステータス保持用 */
			ThreadState_t		threadstatus;
			/** @brief スレッドのアイドリング時間*/
			long				threadIdle;
			/**　@brief 実行されるスレッドの種類 */
			ThreadType_t		threadtype;
			/**　@brief スレッド属性のスタックサイズ */
			long				stacksize;
			/**　@brief スレッドの状態を保持 */
			long				threadCondition;
			/* ***************************************************************
			 * プライベートメソッド
			 * ***************************************************************/
			bool	push(void * FuncQue);
			bool	pop();
			bool	empty();
		public:
			/* ***************************************************************
			 * パブリックメンバ変数
			 * ***************************************************************/
			/**　@brief ミューテックス管理用オブジェクト */
			Mutex	  mutex;
			/* ***************************************************************
			 * コンストラクタ/デストラクタ
			 * ***************************************************************/
			ThreadCall(void);
			virtual ~ThreadCall(void);
			/* ***************************************************************
			 * フレンドメソッド
			 * ***************************************************************/
			friend void *	callBackFunction(void * lpvData);
			/* ***************************************************************
			 * 仮想メソッド
			 * ***************************************************************/
			virtual bool	onFunction(void * Data);
			virtual bool	onFunction();
			/* ***************************************************************
			 * パブリックメソッド
			 * ***************************************************************/
			bool			signalRunning();
			bool			setFunction(void * Data=NULL);
			bool			setFunction(ThreadFunction *Func);
			void			stop();
			bool			start();
			void			getThreadId(pthread_t *thrteadid);
			ThreadState_t	getThreadStatus();
			bool			isThreadRunning(long dwTimeout=0);
			long			getObjectCondition();
			void			setThreadType(
								ThreadType_t	type = TH_TYP_EVENTDRIVEN
							,	long			idle = 100
											);
			void			setIdle(long idle=100);
			void			setStackSize(int size);
			int				getStackSize();
			void			setMaxThread(int maxsize);
			unsigned int	getThreadFunctions();
	};
}
#endif
