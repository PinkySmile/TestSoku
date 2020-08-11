//
// Created by Gegel85 on 05/06/2020.
//

#ifndef SOKUTEST_EXCEPTIONS_HPP
#define SOKUTEST_EXCEPTIONS_HPP


#include <string>

namespace Soku
{
	class BaseException : public std::exception {
	private:
		std::string _msg;

	public:
		explicit BaseException(const std::string &&msg) : _msg(msg) {}
		const char *what() const noexcept override { return this->_msg.c_str(); }
	};

	class GameNotStartedException : public BaseException {
	public:
		explicit GameNotStartedException(const std::string &&msg) :
			BaseException(static_cast<const std::string &&>(msg))
		{}
	};

	class GameAlreadyStartedException : public BaseException {
	public:
		explicit GameAlreadyStartedException(const std::string &&msg) :
			BaseException(static_cast<const std::string &&>(msg))
		{}
	};

	class SpectatorsNotAllowedException : public BaseException {
	public:
		explicit SpectatorsNotAllowedException(const std::string &&msg) :
			BaseException(static_cast<const std::string &&>(msg))
		{}
	};

	class UnknownErrorException : public BaseException {
	public:
		explicit UnknownErrorException(const std::string &&msg) :
			BaseException(static_cast<const std::string &&>(msg))
		{}
	};

	class InvalidHandShakeException : public BaseException {
	public:
		explicit InvalidHandShakeException(const std::string &&msg) :
			BaseException(static_cast<const std::string &&>(msg))
		{}
	};

	class SocketNotReadyException : public BaseException {
	public:
		SocketNotReadyException(): BaseException("The socket is not ready") {};
	};

	class DisconnectedException : public BaseException {
	public:
		DisconnectedException(): BaseException("The socket is not connected") {};
	};

	class SocketErrorException : public BaseException {
	public:
		SocketErrorException(): BaseException("The socket encountered an unexpected error") {};
	};
}


#endif //SOKUTEST_EXCEPTIONS_HPP
