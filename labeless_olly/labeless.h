/* Labeless
 * by Aliaksandr Trafimchuk
 *
 * Source code released under
 * Creative Commons BY-NC 4.0
 * http://creativecommons.org/licenses/by-nc/4.0
 */

#pragma once

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <sstream>
#include <WinSock2.h>

struct Request
{
	uint64_t		id = 0;
	std::string		script;
	std::string		scriptExternObj;
	std::string		params;

	std::string		result;
	std::string		binaryResult;
	std::string		error;

	bool			finished = false;
	bool			background = false;
};

struct ClientData
{
	std::string				peer;
	WORD					peerPort = 0;
	SOCKET					s = INVALID_SOCKET;
	std::stringstream		netBuff;

	std::recursive_mutex	commandsLock;
	std::deque<Request>		commands;

	//std::recursive_mutex	resultLock;
	//std::string				result;

	std::recursive_mutex	stdOutLock;
	std::stringstream		stdOut;

	std::recursive_mutex	stdErrLock;
	std::stringstream		stdErr;

	//std::recursive_mutex	binaryResultLock;
	//std::string				binaryResult;

	//std::recursive_mutex	lock; // this lock

	Request* find(uint64_t jobId);
	bool remove(uint64_t jobId);
};

typedef std::lock_guard<std::recursive_mutex> recursive_lock_guard;


class Labeless
{
	Labeless();
	Labeless(const Labeless&) = delete;
	Labeless& operator=(const Labeless&) = delete;
public:
	static Labeless& instance();
	inline void setHInstance(HINSTANCE hInst) { m_hInst = hInst; }
	inline HINSTANCE hInstance() const { return m_hInst; }

	bool init();
	bool destroy();

	void stopServer();
	bool startServer();

	static WORD defaultPort() { return 3852; }
	inline void setPort(WORD wPort) { m_Port = wPort; }
	inline void setFilterIP(const std::string ip) { m_FilterIP = ip; }
	WORD port() const { return m_Port; }
	std::string filterIP() const { return m_FilterIP; }
	static std::string lastChangeTimestamp();

	void onSetPortRequested();
	void onSetIPFilter();

	inline ClientData& clientData() { return m_Rpc; }
	inline const ClientData& clientData() const { return m_Rpc; }

private:
	bool initPython();
	void destroyPython();

	HWND createWindow();

	static bool bindAndListenSock(SOCKET& rv, WORD& wPort, const std::string& ip = std::string());
	static void WINAPI serverThread(Labeless* ll);

	static LRESULT CALLBACK helperWinProc(HWND hw, UINT msg, WPARAM wp, LPARAM lp);
	bool onCommandReceived(const std::string& command, const std::string& scriptExternObj);
	bool onCommandReceived(ClientData& cd);
	void onPortChanged();

	static bool onClientSockAccept(SOCKET sock, ClientData& cd);
	static bool onClientSockRead(ClientData& cd);
	static bool onClientSockBufferReceived(ClientData& cd, const std::string& rawCommand);
	static bool onClientSockClose(ClientData& cd);

	static std::string storeBadCommand(const std::string& rawCommand);

private:
	typedef std::shared_ptr<std::thread> ThreadPtr;

	HINSTANCE				m_hInst;
	WORD					m_Port;
	HWND					m_LogList;

	std::string				m_FilterIP;

	std::recursive_mutex	m_ThreadLock;
	ThreadPtr				m_Thread;
	static std::atomic_bool	m_ServerEnabled;

	ClientData				m_Rpc;
};

