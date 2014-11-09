/* ***************************************************************************
 * @file		VSTDThreadCall.cpp
 * @brief		スレッドイベント呼び出し用 Class
 * @see		VSTD::ThreadFunction / VSTD::Condition
 * @author	Sebastian
 * @date		2009/7/16
 * @version	1.0
 *  
 * 
 * @par 更新履歴：
 * - 2009/07/16	Sebastian 新規作成
 * ***************************************************************************/
#include "VSTDThreadCall.hpp"

int nanosleep(const struct timespec * rqtp , struct timespec * rmtp);

namespace VSTD
{
	void Sleep (unsigned int milliSecond)
	{
		unsigned int second;
		struct timespec interval;
		struct timespec remainder;
		second = milliSecond *1000000;
		interval.tv_sec= 0;
		interval.tv_nsec=second;
		nanosleep(&interval,&remainder);
	}
	/* ***********************************************************************
	 *
	 * コンストラクタ/デストラクタ
	 *
	 *************************************************************************/
	/**
	 * @brief		ThreadCallのコンストラクタ
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	ThreadCall::ThreadCall(void)
	{
		running				= false;
		threadId				= 0L;
		threadstatus			= TH_STAT_DOWN;
		threadIdle			= 100;
		threadQueue			= NULL;
		ProcessQueue			= NULL;
		threadQueDepath		= MAX_THREAD;
		threadtype			= TH_TYP_EVENTDRIVEN;
		stacksize				= 100;
		currentThreadQueue	= 0;
		threadCondition		= TH_ERR_NOERROR;
		threadQueue			= new void * [MAX_THREAD];
		try
		{

			if (!threadQueue)
			{
				threadCondition	|=	TH_ERR_MEMORY_ERR	;
				threadstatus		=	TH_STAT_FAULT;
				return;
			}
			if (!mutex.created)
			{
				threadCondition	|=	TH_ERR_MUTEX_CREATE;
				threadstatus		=	TH_STAT_FAULT;
				std::string desc = "[ThreadCall]:ミューテックスが正常に作成出来ませんでした。";
				throw desc;
				return;
			}
			if (!condition.created)
			{
				threadCondition	|=	TH_ERR_COND_CREATE;
				threadstatus		=	TH_STAT_FAULT;
				std::string desc = "[ThreadCall]:条件変数が正常に作成出来ませんでした。";
				throw desc;
				return;
			}
			/* 開始呼出 */
			start();
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		ThreadCallのデストラクタ
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	ThreadCall::~ThreadCall(void)
	{
		void * result;
		try
		{
			/* 実行中の場合は停止 */
			if (running)
			{
				stop();
				pthread_join(threadhandle,&result);
			}
			/* スレッドファンクションをクリア */
			delete [] threadQueue;
		}
		catch(...)
		{
			throw;
		}
	}
	/* ***********************************************************************
	 *
	 * フレンドメソッド
	 *
	 * ***********************************************************************/
	/**
	 * @brief		callBakFunction
	 * 				スレッドの実態
	 * @note		startメソッド内にてpthread_creatにてスレッドとして実体化
	 * 				され実行されるメソッドです。
	 *				引数としてVSTDFunctionCallオブジェクトの実態を引き渡す為
	 * 				スレッドファンクションとして積み上げられたファンクションを
	 * 				コールバックして呼び出し逐次実行を行います。
	 * @param		[in]ThreadCall：ThreadCallオブジェクトが指定されます。
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	void * callBackFunction (void * threadCall)
	{
		/* *******************************************************************
		 * ThreadCallの各種設定を変更
		 * *******************************************************************/
		ThreadCall *	pThread;
		ThreadType_t		Type;
		pThread = (ThreadCall *)threadCall;
		pThread->mutex.lock();
		pThread->threadstatus = TH_STAT_WAIT;
		pThread->running	= true;
		pThread->threadId	= pthread_self();
		pThread->mutex.unlock();
		try
		{
			/* *******************************************************************
			 * 待機しているスレッドファンクションを実行
			 * *******************************************************************/
			while (true)
			{
				Type = pThread->threadtype;
				/* ***************************************************************
				 * スレッド種別がイベントドリブンタイプである
				 * ***************************************************************/
				if (Type == TH_TYP_EVENTDRIVEN)
				{
					/* ***********************************************************
					 * 条件変数が待機状態出ない
					 * ***********************************************************/
					if (!pThread->condition.wait())
					{
						break;
					}
				}
				/* ***************************************************************
				 * 待機状態のスレッドファンクションを逐次実行
				 * ***************************************************************/
				if (!pThread->signalRunning())
				{
					break;
				}
				/* ***************************************************************
				 * イベントドリブン型の場合は条件変数をリセット
				 * ***************************************************************/
				if (Type == TH_TYP_EVENTDRIVEN)
				{
					pThread->condition.reset();
				}
				/* ***************************************************************
				 * インターバル型の場合はアイドリング
				 * ***************************************************************/
				if (pThread->threadtype == TH_TYP_INTERVAL)
				{
					Sleep(pThread->threadIdle);
				}
			}
			/* *******************************************************************
			 * スレッドのシャットダウン処理
			 * *******************************************************************/
			pThread->mutex.lock();
			pThread->threadstatus = TH_STAT_DOWN;
			pThread->running = false;
			pThread->mutex.unlock();
			return (void *)0;
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		onFunction
	 * 				ラッピング用仮想ファンクション
	 * @note		ThreadCallをラッピングしsetFunctionをコールする事で
	 * 				ラッピングしたonFunctionをスレッドとして実行します。
	 * 				また、引数Dataに値を与える事でスレッドに対して値を渡す事も
	 * 				可能となります。
	 * @return		成否を返却します。
	 * @retval		true ： 成功
	 * @retval		false： 失敗
	 * @author		Sebastian
	 * @date		2009/7/16
	 */
	bool ThreadCall::onFunction (void * Data)
	{
		bool retVal;
		try
		{
			ThreadFunction *Func = (ThreadFunction *)Data;
			Func->setStatus(THFUNC_STATE_PROCESSED);
			retVal = Func->Function();
			Func->setStatus(THFUNC_STATE_COMLETED);
			return retVal;
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		onFunction
	 * 				ラッピング用仮想ファンクション
	 * @note		ThreadCallをラッピングしsetFunctionをコールする事で
	 * 				ラッピングしたonFunctionをスレッドとして実行します。
	 * @return	成否を返却します。
	 * @retval	true ： 成功
	 * @retval	false： 失敗
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	bool ThreadCall::onFunction()
	{
		return true;
	}
	/**
	 * @brief		setFunction
	 * 				ThreadFunctionの積み上げ
	 * @note		待ち行列にThreadFunctionオブジェクトを追加し
	 * 				条件変数にて待機状態を設定します。
	 * @param[in]	Func：追加するTHreadFunctionオブジェクトを指定
	 * @return	成否を返却します。
	 * @retval	true ： 成功
	 * @retval	false： 失敗
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	bool ThreadCall::setFunction(ThreadFunction *Func)
	{
		pthread_t id;
		try
		{
			/* ***************************************************************
			 * 実行種別がイベントドリブン型である事を確認
			 * ***************************************************************/
			mutex.lock();
			if (threadtype != TH_TYP_EVENTDRIVEN)
			{
				mutex.unlock();
				threadCondition	|= 	TH_ERR_ILLEGAL_USE_COND;
				threadstatus		=	TH_STAT_FAULT;
				return false;
			}
			mutex.unlock();
			/* ***************************************************************
			 * スレッドファンクションをキューにスタックする
			 * ***************************************************************/
			/* スレッドIDの取得 */
			getThreadId(&id);
			/* スレッドファンクションにスレッドIDを指定 */
			Func->setThreadId(&id);
			/* スレッドファンクションの待ち行列にキューを追加 */
			if (!push((void *)Func))
			{
				return false;
			}
			/* ***************************************************************
			 * スレッドファンクションをシグナル待ち待機状態に指定
			 * ***************************************************************/
			Func->setStatus(THFUNC_STATE_WAITING);
			condition.set();
			return true;
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		setFunction
	 * 				ThreadFunction用のデータの積み上げ
	 * @note		待ち行列にThreadFunctionオブジェクトが
	 * 				使用するデータを追加して積み上げます。
	 * @param[in]	Data：追加するThreadFunctionオブジェクトを指定
	 * @return	成否を返却します。
	 * @retval	true ： 成功
	 * @retval	false： 失敗
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	 bool ThreadCall::setFunction(void * Data)
	{
		 try
		 {
			mutex.lock();
			if (threadtype != TH_TYP_EVENTDRIVEN)
			{
				mutex.unlock();
				threadCondition |= TH_ERR_ILLEGAL_USE_COND;
				threadstatus = TH_STAT_FAULT;
				return false;
			}
			mutex.unlock();
			if (! push(Data))
			{
				return false;
			}
			condition.set();
			return true;
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		signalRunning
	 * @note		待ち行列に積まれたThreadFunctionのキューを逐次実行
	 * @return	成否を返却します。
	 * @retval	true ： 成功
	 * @retval	false： 失敗
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	bool ThreadCall::signalRunning()
	{
		try
		{
			/* ミューテックスを取得 */
			mutex.lock();
			/* ステータスを実行中に指定 */
			threadstatus = TH_STAT_BUSY;
			/* 多重実行回避処理 */
			if (!running)
			{
				threadstatus = TH_STAT_SHUTDOWN;
				mutex.unlock();
				return false;
			}
			/* ミューテックスの開放 */
			mutex.unlock();
			/* 待機しているキューが空で無い場合 */
			if (!empty())
			{
				/* 条件変数が空になるまで実行 */
				while (!empty())
				{
					pop();
					if (!onFunction(ProcessQueue))
					{
						mutex.lock();
						ProcessQueue = NULL;
						threadstatus = TH_STAT_SHUTDOWN;
						mutex.unlock();
						return false;
					}
				}
				mutex.lock();
				ProcessQueue = NULL;
				threadstatus = TH_STAT_WAIT;
			}
			/* 待機しているキューが空の場合 */
			else
			{
				if (!onFunction())
				{
					mutex.lock();
					threadstatus = TH_STAT_SHUTDOWN;
					mutex.unlock();
					return false;
				}
				mutex.lock();
				threadstatus = TH_STAT_WAIT;
			}
			mutex.unlock();
			return true;
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		getThreadFunctions
	 * 				現在待機中のThreadFunctionの数を取得します
	 * @return	ThreadFunctionの数を返却します。
	 * @author	Sebastian
	 * @date		2009/7/13
	 */
	unsigned int ThreadCall::getThreadFunctions()
	{
		unsigned int chEventsWaiting;
		try
		{
			mutex.lock();
			chEventsWaiting = currentThreadQueue;
			mutex.unlock();
			return chEventsWaiting;
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		setThreadtype
	 * 				スレッドの実行種別を変更します。
	 * @param[in]	type：スレッドの実行種別を指定します。
	 * 				Defaultでイベントドリブン型(TH_TYP_EVENTDRIVEN)
	 * @param[in]	idel：スレッドのアイドル時間を指定します。
	 * 				Defaultで100milli秒
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	void ThreadCall::setThreadType(ThreadType_t type , long idle)
	{
		try
		{
			/* ミューテックスの取得 */
			mutex.lock();
			threadIdle = idle;
			if(threadtype == type)
			{
				mutex.unlock();
				return;
			}
			threadtype = type;
			/* ミューテックスの解放 */
			mutex.unlock();
			/* シグナル状態に設定 */
			condition.set();
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		stop
	 * 				スレッド停止。
	 * @note		実行中のスレッドを停止します。
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	void ThreadCall::stop()
	{
		try
		{
			/* 実行フラグの変更 */
			mutex.lock();
			running = false;
			mutex.unlock();
			setFunction();
			/* *******************************************************************
			 * スレッドの終了までアイドリング
			 * *******************************************************************/
			Sleep(threadIdle);
			while(true)
			{
				/* スレッドがシャットダウンするまでアイドリングを続ける */
				mutex.lock();
				if (threadstatus == TH_STAT_DOWN)
				{
					mutex.unlock();
					return;
				}
				mutex.unlock();
				Sleep(threadIdle);
			}
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		setIdel
	 * 				スレッドのアイドリング時間をミリ秒単位で設定します。
	 * @note		スレッドタイプがインターバルのスレッド処理でのみ使用されます。
	 * @param		idol：アイドリング時間をミリ秒にて指定します。
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	void ThreadCall::setIdle(long idle)
	{
		try
		{
			mutex.lock();
			threadIdle = idle;
			mutex.unlock();
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		setStackSize
	 * 				スレッドのスタックサイズを設定
	 * @author	Sebastian
	 * @date		2009/7/14
	 */
	void ThreadCall::setStackSize(int size)
	{
		try
		{
			pthread_attr_setstacksize(&thread_attr,size);
			stacksize = size;
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		setStackSize
	 * 				スレッドのスタックサイズを設定
	 * @author	Sebastian
	 * @date		2009/7/14
	 */
	int ThreadCall::getStackSize()
	{

		try
		{
			pthread_attr_setstacksize(&thread_attr,stacksize);
			return stacksize;
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		start
	 * 				スレッドの開始
	 * @note		スレッド初期化、作成し実行します。
	 * @return	成否を返却します。
	 * @retval	true ： 成功
	 * @retval	false： 失敗
	 * @author	Sebastian
	 * @date		2009/7/14
	 */
	bool ThreadCall::start()
	{
		int				result;
		std::string 	desc;
		try
		{
			/* *******************************************************************
			 * コールバックスレッド生成処理
			 * *******************************************************************/
			/* 実行確認 */
			mutex.lock();
			if (running)
			{
				mutex.unlock();
				return true;
			}
			mutex.unlock();
			/* スレッドコンディションの設定 */
			if (threadCondition & TH_ERR_THREAD_CREATE)
			{
				threadCondition = threadCondition ^ TH_ERR_THREAD_CREATE;
			}
			/* スレッド属性の初期化 */
			pthread_attr_init(&thread_attr);
			/* スタックサイズの設定 */
			if (stacksize != 0)
			{
				pthread_attr_setstacksize(&thread_attr,stacksize);
			}
			/* コールバックファンクションを用いてスレッドを作成 */
			result = pthread_create(&threadhandle
								,&thread_attr,callBackFunction,(void *)this);
			/* *******************************************************************
			 * エラー処理
			 * *******************************************************************/
			if (result != 0)
			{
				threadCondition	|=	TH_ERR_THREAD_CREATE;
				threadstatus		=	TH_STAT_FAULT;
				switch(result)
				{
				case EINVAL:
					desc = "[ThreadCall]:無効なスレッド属性が指定されました。";
					throw desc;
					break;
				case EAGAIN:
					desc = "[ThreadCall]:新しいスレッドを生成する為のシステム資源がありません。";
					throw desc;
					break;
				case EPERM:
					desc = "[ThreadCall]:スレッド属性が適切に初期化されていません。";
					throw desc;
					break;
				default:
					desc = "[ThreadCall]:原因不明のエラー発生しました。";
					throw desc;
					break;
				}
				return false;
			}
			return true;
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		getThreadStatus
	 * 				スレッドステータス取得
	 * @return	現在のスレッド状態を返却します。
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	ThreadState_t ThreadCall::getThreadStatus()
	{
		try
		{
			ThreadState_t	retval;
			mutex.lock();
			retval = threadstatus;
			mutex.unlock();
			return retval;
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		isThreadRunning
	 * 				スレッドの実行中確認する
	 * @note		指定したタイムアウトが訪れるまで
	 * 				アイドリングとミューテックスの取得を繰り返しながら
	 * 				実行中フラグの取得を行う。
	 * @param[in]	timeout：タイムアウト時間を指定
	 * @return	確認結果を返却
	 * @retval	true ：スレッド実行中
	 * @retval	false：スレッドは実行していない
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	bool ThreadCall::isThreadRunning(long timeout)
	{
		long total = 0;
		try
		{
			/* *******************************************************************
			 * アイドリングしながら実行中フラグの取得
			 * *******************************************************************/
			while(true)
			{
				/* タイムアウト処理 */
				if (total > timeout && timeout > 0)
				{
					return false;
				}
				/* ミューテックスの取得 */
				mutex.lock();
				if (running)
				{
					/* 実行中と判断 */
					mutex.unlock();
					return true;
				}
				/* 現在のアイドリング時間の総計を計算 */
				total += threadIdle;
				/* ミューテックスの解放 */
				mutex.unlock();
				/* アイドリング */
				Sleep(threadIdle);
			}
			return false;
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		getThreadId
	 * 				スレッドIDの取得
	 * @note		スレッドファンクションのスレッドIDを取得します。
	 * @param[in]	threadid : スレッドIDを参照渡しします。
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	void ThreadCall::getThreadId(pthread_t *threadid)
	{
		try
		{
			memcpy(threadid,&threadId,sizeof(pthread_t));
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		getObjectCondition
	 * 				ThreadCallオブジェクトの状態を返却
	 * @return	オブジェクトの状態をthreaderror_tにて返却
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	long ThreadCall::getObjectCondition()
	{
		return threadCondition;
	}
	/* ***************************************************************************
	 *
	 * プライベートメソッド
	 *
	 ****************************************************************************/
	/**
	 * @brief		empty
	 * 				待機中のスレッドファンクションの有無を確認
	 * @return	スレッドファンクションの有無を返却
	 * @retval	true:現在待機中のものは存在しない
	 * @retval	false:待機中である
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	bool ThreadCall::empty()
	{
		try
		{
			mutex.lock();
			if (currentThreadQueue <= 0)
			{
				mutex.unlock();
				return true;
			}
			mutex.unlock();
			return false;
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		push
	 * 				ThreadFunctionを追加
	 * @param[in]	FuncQue 追加するThreadFunctionを指定
	 * @return	処理の成否を返却
	 * @retval	true:成功
	 * @retval	false:失敗
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	bool ThreadCall::push(void * FuncQue)
	{
		try
		{
			if (!FuncQue) return true;

			mutex.lock();
			if (currentThreadQueue + 1 >= threadQueDepath)
			{
				mutex.unlock();
				return false;
			}
			threadQueue[currentThreadQueue++] = FuncQue;
			mutex.unlock();
			return true;
		}
		catch(...)
		{
			throw;
		}
	}
	/**
	 * @brief		pop
	 * 				ThreadFunctionを取得
	 * @note		pushメソッドにて積み上げられたThreadFunctionを
	 * 				末尾から一つ取り出す。
	 * @return	処理の成否を返却
	 * @retval	true:成功
	 * @retval	false:失敗
	 * @author	Sebastian
	 * @date		2009/7/16
	 */
	bool ThreadCall::pop()
	{
		try
		{
			mutex.lock();
			if (currentThreadQueue-1 < 0)
			{
				currentThreadQueue = 0;
				mutex.unlock();
				return false;
			}
			currentThreadQueue--;
			ProcessQueue = threadQueue[currentThreadQueue];
			mutex.unlock();
			return true;
		}
		catch(...)
		{
			throw;
		}
	}
}
