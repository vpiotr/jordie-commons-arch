/*
 * cpp_string_io.h
 *
 *  Created on: Jul 28, 2018
 *      Author: piotr
 */

#ifndef CPP_STRING_IO_H_
#define CPP_STRING_IO_H_

#include <string>
#include <sstream>
#include <type_traits>
#include <ostream>

namespace csi {

	namespace details {

		class printable_base {
			public:
			virtual std::string as_string() const = 0;
			virtual ~printable_base() {}

			virtual std::ostream& print(std::ostream& out) const
			{
				out << this->as_string();
				return out;
			}

			friend std::ostream& operator<<(std::ostream &out, const printable_base &b)
			{
				b.print(out);
				return out;
			}
		};


		template<typename T>
		std::string _to_string_as_int(T value, std::true_type)
		{
			return std::to_string(value);
		}

		template<typename T>
		std::string _to_string_as_float(T value, std::true_type)
		{
			return std::to_string(value);
		}

		template<typename T>
		typename std::enable_if<!std::is_base_of<printable_base, T>::value, std::string>::type
		_to_string_as_printable(T value) {
			std::ostringstream	out;
			out << value;
			return(out.str());
		}

		std::string _to_string_as_printable(const printable_base &value) {
			return value.as_string();
		}

		template<typename T>
		std::string _to_string_as_float(T value, std::false_type)
		{
			return _to_string_as_printable(value);
		}


		template<typename T>
		std::string _to_string_as_int(T value, std::false_type)
		{
			return _to_string_as_float(value, std::is_floating_point<T>());
		}

	}

	template<typename T>
	std::string as_string(T value) {
		return details::_to_string_as_int(value, std::is_integral<T>());
	}

	std::string as_string(const std::string &value) {
		return value;
	}

	template<typename T>
	std::string as_string(const T *begin, const T *end) {
		std::ostringstream	out;
		bool next = false;
		out << "[";
		for(auto it = begin; it != end; ++it) {
			if (next) {
				out << ", ";
			} else {
				next = true;
			}
			out << as_string(*it);
		}
		out << "]";

		return(out.str());
	}

	class printable: public details::printable_base {
	public:
	};

}



#endif /* CPP_STRING_IO_H_ */
