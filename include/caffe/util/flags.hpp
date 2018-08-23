
#ifndef _FLAGS_HPP_
#define _FLAGS_HPP_

#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
#include <sstream>
#include <inttypes.h>
#include <stdarg.h>
#include <assert.h>
typedef std::string String;

struct Param {
	enum {
		INT = 0, BOOLEAN = 1, REAL = 2, STRING = 3, MAT = 4, MAT_VECTOR = 5, ALGORITHM = 6, FLOAT = 7,
		UNSIGNED_INT = 8, UINT64 = 9, UCHAR = 11, SCALAR = 12
	};
};

template<typename _Tp> struct ParamType {};
#define PARAMTYPEDEF(a, b) template<> struct ParamType<a> { \
	typedef const a& const_param_type; \
	typedef a member_type; \
	enum { type = Param::b }; \
}

PARAMTYPEDEF(int, INT);
PARAMTYPEDEF(bool, BOOLEAN);
PARAMTYPEDEF(double, REAL);
PARAMTYPEDEF(String, STRING);
PARAMTYPEDEF(float, FLOAT);
PARAMTYPEDEF(unsigned int, UNSIGNED_INT);
PARAMTYPEDEF(int64_t, UINT64);
PARAMTYPEDEF(unsigned char, UCHAR);



struct CommandLineParserParams
{
public:
	String help_message;
	String def_value;
	std::vector<String> keys;
	int number;
};



/** @brief Designed for command line parsing

The sample below demonstrates how to use CommandLineParser:
@code
    CommandLineParser parser(argc, argv, keys);
    parser.about("Application name v1.0.0");

    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }

    int N = parser.get<int>("N");
    double fps = parser.get<double>("fps");
    String path = parser.get<String>("path");

    use_time_stamp = parser.has("timestamp");

    String img1 = parser.get<String>(0);
    String img2 = parser.get<String>(1);

    int repeat = parser.get<int>(2);

    if (!parser.check())
    {
        parser.printErrors();
        return 0;
    }
@endcode

### Keys syntax

The keys parameter is a string containing several blocks, each one is enclosed in curly braces and
describes one argument. Each argument contains three parts separated by the `|` symbol:

-# argument names is a space-separated list of option synonyms (to mark argument as positional, prefix it with the `@` symbol)
-# default value will be used if the argument was not provided (can be empty)
-# help message (can be empty)

For example:

@code{.cpp}
    const String keys =
        "{help h usage ? |      | print this message   }"
        "{@image1        |      | image1 for compare   }"
        "{@image2        |<none>| image2 for compare   }"
        "{@repeat        |1     | number               }"
        "{path           |.     | path to file         }"
        "{fps            | -1.0 | fps for output video }"
        "{N count        |100   | count of objects     }"
        "{ts timestamp   |      | use time stamp       }"
        ;
}
@endcode

Note that there are no default values for `help` and `timestamp` so we can check their presence using the `has()` method.
Arguments with default values are considered to be always present. Use the `get()` method in these cases to check their
actual value instead.

String keys like `get<String>("@image1")` return the empty string `""` by default - even with an empty default value.
Use the special `<none>` default value to enforce that the returned string must not be empty. (like in `get<String>("@image2")`)

### Usage

For the described keys:

@code{.sh}
    # Good call (3 positional parameters: image1, image2 and repeat; N is 200, ts is true)
    $ ./app -N=200 1.png 2.jpg 19 -ts

    # Bad call
    $ ./app -fps=aaa
    ERRORS:
    Parameter 'fps': can not convert: [aaa] to [double]
@endcode
 */
class CommandLineParser
{
public:

    /** @brief Constructor

    Initializes command line parser object

    @param argc number of command line arguments (from main())
    @param argv array of command line arguments (from main())
    @param keys string describing acceptable command line parameters (see class description for syntax)
    */
    CommandLineParser(int argc, const char* const argv[], const String& keys);

    /** @brief Copy constructor */
    CommandLineParser(const CommandLineParser& parser);

    /** @brief Assignment operator */
    CommandLineParser& operator = (const CommandLineParser& parser);

    /** @brief Destructor */
    ~CommandLineParser();

    /** @brief Returns application path

    This method returns the path to the executable from the command line (`argv[0]`).

    For example, if the application has been started with such a command:
    @code{.sh}
    $ ./bin/my-executable
    @endcode
    this method will return `./bin`.
    */
    String getPathToApplication() const;

    /** @brief Access arguments by name

    Returns argument converted to selected type. If the argument is not known or can not be
    converted to selected type, the error flag is set (can be checked with @ref check).

    For example, define:
    @code{.cpp}
    String keys = "{N count||}";
    @endcode

    Call:
    @code{.sh}
    $ ./my-app -N=20
    # or
    $ ./my-app --count=20
    @endcode

    Access:
    @code{.cpp}
    int N = parser.get<int>("N");
    @endcode

    @param name name of the argument
    @param space_delete remove spaces from the left and right of the string
    @tparam T the argument will be converted to this type if possible

    @note You can access positional arguments by their `@`-prefixed name:
    @code{.cpp}
    parser.get<String>("@image");
    @endcode
     */
    template <typename T>
    T get(const String& name, bool space_delete = true) const
    {
        T val = T();
        getByName(name, space_delete, ParamType<T>::type, (void*)&val);
        return val;
    }

    /** @brief Access positional arguments by index

    Returns argument converted to selected type. Indexes are counted from zero.

    For example, define:
    @code{.cpp}
    String keys = "{@arg1||}{@arg2||}"
    @endcode

    Call:
    @code{.sh}
    ./my-app abc qwe
    @endcode

    Access arguments:
    @code{.cpp}
    String val_1 = parser.get<String>(0); // returns "abc", arg1
    String val_2 = parser.get<String>(1); // returns "qwe", arg2
    @endcode

    @param index index of the argument
    @param space_delete remove spaces from the left and right of the string
    @tparam T the argument will be converted to this type if possible
     */
    template <typename T>
    T get(int index, bool space_delete = true) const
    {
        T val = T();
        getByIndex(index, space_delete, ParamType<T>::type, (void*)&val);
        return val;
    }

    /** @brief Check if field was provided in the command line

    @param name argument name to check
    */
    bool has(const String& name) const;

    /** @brief Check for parsing errors

    Returns false if error occurred while accessing the parameters (bad conversion, missing arguments,
    etc.). Call @ref printErrors to print error messages list.
     */

    /** @brief Set the about message

    The about message will be shown when @ref printMessage is called, right before arguments table.
     */
    void about(const String& message);

    /** @brief Print help message

    This method will print standard help message containing the about message and arguments description.

    @sa about
    */
    void printMessage() const;

    /** @brief Print list of errors occurred

    @sa check
    */
    void printErrors() const;

protected:
    void getByName(const String& name, bool space_delete, int type, void* dst) const;
    void getByIndex(int index, bool space_delete, int type, void* dst) const;

	String path_to_app;
	String app_name;
	String about_message;

	std::vector<CommandLineParserParams> data;

	std::vector<String> split_range_string(const String& str, char fs, char ss) const;
	std::vector<String> split_string(const String& str, char symbol = ' ', bool create_empty_item = false) const;

	void apply_params(const String& key, const String& value);
	void apply_params(int i, String value);

	void sort_params();

};


String format(const char* fmt, ...)
{
	std::vector<char> buf(1024);

	for (; ; )
	{
		va_list va;
		va_start(va, fmt);
		int bsize = static_cast<int>(buf.size());
		int len = vsnprintf(buf.data(), bsize, fmt, va);
		va_end(va);

		assert(len >= 0 && "Check format string for errors");
		if (len >= bsize)
		{
			buf.resize(len + 1);
			continue;
		}
		buf[bsize - 1] = 0;
		return String(buf.data(), len);
	}
}

static const char* noneValue = "<none>";

static String cat_string(const String& str)
{
	int left = 0, right = (int)str.length();
	while (left < right && str[left] == ' ')
		left++;
	while (right > left && str[right - 1] == ' ')
		right--;
	return left >= right ? String("") : str.substr(left, right - left);
}


static const char* get_type_name(int type)
{
	if (type == Param::INT)
		return "int";
	if (type == Param::BOOLEAN)
		return "bool";
	if (type == Param::UNSIGNED_INT)
		return "unsigned";
	if (type == Param::UINT64)
		return "unsigned long long";
	if (type == Param::FLOAT)
		return "float";
	if (type == Param::REAL)
		return "double";
	if (type == Param::STRING)
		return "string";
	return "unknown";
}

// std::tolower is int->int
static char char_tolower(char ch)
{
	return (char)std::tolower((int)ch);
}
static bool parse_bool(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), char_tolower);
	std::istringstream is(str);
	bool b;
	is >> (str.size() > 1 ? std::boolalpha : std::noboolalpha) >> b;
	return b;
}
#define FLAGS_Error(a, b)  printf(b)
#define FLAGS_Error_(a, b)  printf b

static void from_str(const String& str, int type, void* dst)
{
	std::stringstream ss(str.c_str());
	if (type == Param::INT)
		ss >> *(int*)dst;
	else if (type == Param::BOOLEAN)
	{
		std::string temp;
		ss >> temp;
		*(bool*)dst = parse_bool(temp);
	}
	else if (type == Param::UNSIGNED_INT)
		ss >> *(unsigned*)dst;
	else if (type == Param::UINT64)
		ss >> *(uint64_t*)dst;
	else if (type == Param::FLOAT)
		ss >> *(float*)dst;
	else if (type == Param::REAL)
		ss >> *(double*)dst;
	else if (type == Param::STRING)
		*(String*)dst = str;
#if 0
	else if (type == Param::SCALAR)
	{
		Scalar& scalar = *(Scalar*)dst;
		for (int i = 0; i < 4 && !ss.eof(); ++i)
			ss >> scalar[i];
	}
#endif
	else
		FLAGS_Error(Error::StsBadArg, "unknown/unsupported parameter type");

	if (ss.fail())
	{
		FLAGS_Error_(Error::StsBadArg, ("can not convert: [%s] to [%s]", str.c_str(), get_type_name(type)));
	}
}
//#define this this

void CommandLineParser::getByName(const String& name, bool space_delete, int type, void* dst) const
{
	{
		for (size_t i = 0; i < this->data.size(); i++)
		{
			for (size_t j = 0; j < this->data[i].keys.size(); j++)
			{
				if (name == this->data[i].keys[j])
				{
					String v = this->data[i].def_value;
					if (space_delete)
						v = cat_string(v);

					// the key was neither specified nor has a default value
					if ((v.empty() && type != Param::STRING) || v == noneValue) {
						//printf("Missing parameter: '" + name + "'\n");
						return;
					}

					from_str(v, type, dst);
					return;
				}
			}
		}
	}
#if 0
	if (0) {
		this->error = true;
		this->error_message = this->error_message + "Parameter '" + name + "': " + e.err + "\n";
		return;
	}
	FLAGS_Error_(Error::StsBadArg, ("undeclared key '%s' requested", name.c_str()));
#endif
	return;
}


void CommandLineParser::getByIndex(int index, bool space_delete, int type, void* dst) const
{

	{
		for (size_t i = 0; i < this->data.size(); i++)
		{
			if (this->data[i].number == index)
			{
				String v = this->data[i].def_value;
				if (space_delete == true) v = cat_string(v);

				// the key was neither specified nor has a default value
				if ((v.empty() && type != Param::STRING) || v == noneValue) {
					
					// error_message = this->error_message + format("Missing parameter #%d\n", index);
					return;
				}
				from_str(v, type, dst);
				return;
			}
		}
	}
#if 0
	CV_CATCH(Exception, e)
	{
		this->error = true;
		this->error_message = this->error_message + format("Parameter #%d: ", index) + e.err + "\n";
		return;
	}
	FLAGS_Error_(Error::StsBadArg, ("undeclared position %d requested", index));
#endif
}

static bool cmp_params(const CommandLineParserParams & p1, const CommandLineParserParams & p2)
{
	if (p1.number < p2.number)
		return true;

	if (p1.number > p2.number)
		return false;

	return p1.keys[0].compare(p2.keys[0]) < 0;
}

CommandLineParser::CommandLineParser(int argc, const char* const argv[], const String& keys)
{
	// path to application
	size_t pos_s = String(argv[0]).find_last_of("/\\");
	if (pos_s == String::npos)
	{
		this->path_to_app = "";
		this->app_name = String(argv[0]);
	}
	else
	{
		this->path_to_app = String(argv[0]).substr(0, pos_s);
		this->app_name = String(argv[0]).substr(pos_s + 1, String(argv[0]).length() - pos_s);
	}

	// parse keys
	std::vector<String> k = this->split_range_string(keys, '{', '}');

	int jj = 0;
	for (size_t i = 0; i < k.size(); i++)
	{
		std::vector<String> l = this->split_string(k[i], '|', true);
		CommandLineParserParams p;
		p.keys = this->split_string(l[0]);
		p.def_value = l[1];
		p.help_message = cat_string(l[2]);
		p.number = -1;
		if (p.keys.size() <= 0)
		{
			// this->error_message = "Field KEYS could not be empty\n";
		}
		else
		{
			if (p.keys[0][0] == '@')
			{
				p.number = jj;
				jj++;
			}

			this->data.push_back(p);
		}
	}

	// parse argv
	jj = 0;
	for (int i = 1; i < argc; i++)
	{
		String s(argv[i]);
		bool hasSingleDash = s.length() > 1 && s[0] == '-';

		if (hasSingleDash)
		{
			bool hasDoubleDash = s.length() > 2 && s[1] == '-';
			String key = s.substr(hasDoubleDash ? 2 : 1);
			String value = "true";
			size_t equalsPos = key.find('=');

			if (equalsPos != String::npos) {
				value = key.substr(equalsPos + 1);
				key = key.substr(0, equalsPos);
			}
			this->apply_params(key, value);
		}
		else
		{
			this->apply_params(jj, s);
			jj++;
		}
	}

	this->sort_params();
}

CommandLineParser::~CommandLineParser()
{
}

CommandLineParser::CommandLineParser(const CommandLineParser& parser)
{
}

void CommandLineParser::apply_params(const String& key, const String& value)
{
	for (size_t i = 0; i < data.size(); i++)
	{
		for (size_t k = 0; k < data[i].keys.size(); k++)
		{
			if (key.compare(data[i].keys[k]) == 0)
			{
				data[i].def_value = value;
				break;
			}
		}
	}
}

void CommandLineParser::apply_params(int i, String value)
{
	for (size_t j = 0; j < data.size(); j++)
	{
		if (data[j].number == i)
		{
			data[j].def_value = value;
			break;
		}
	}
}

void CommandLineParser::sort_params()
{
	for (size_t i = 0; i < data.size(); i++)
	{
		std::sort(data[i].keys.begin(), data[i].keys.end());
	}

	std::sort(data.begin(), data.end(), cmp_params);
}

String CommandLineParser::getPathToApplication() const
{
	return this->path_to_app;
}

bool CommandLineParser::has(const String& name) const
{
	for (size_t i = 0; i < this->data.size(); i++)
	{
		for (size_t j = 0; j < this->data[i].keys.size(); j++)
		{
			if (name == this->data[i].keys[j])
			{
				const String v = cat_string(this->data[i].def_value);
				return !v.empty() && v != noneValue;
			}
		}
	}

	//FLAGS_Error_(Error::StsBadArg, ("undeclared key '%s' requested", name.c_str()));
	return false;
}


void CommandLineParser::printMessage() const
{
	if (this->about_message != "")
		printf("%s\n", this->about_message.c_str());

	printf("Usage: %s [params] ", this->app_name.c_str());

	for (size_t i = 0; i < this->data.size(); i++)
	{
		if (this->data[i].number > -1)
		{
			String name = this->data[i].keys[0].substr(1, this->data[i].keys[0].length() - 1);
			printf("%s ", name.c_str());
		}
	}

	printf("\n\n");

	for (size_t i = 0; i < this->data.size(); i++)
	{
		if (this->data[i].number == -1)
		{
			printf("\t");
			for (size_t j = 0; j < this->data[i].keys.size(); j++)
			{
				String k = this->data[i].keys[j];
				if (k.length() > 1)
				{
					printf("--");
				}
				else
				{
					printf("-");
				}
				printf("%s", k.c_str());

				if (j != this->data[i].keys.size() - 1)
				{
					printf(", ");
				}
			}
			String dv = cat_string(this->data[i].def_value);
			if (dv.compare("") != 0)
			{
				printf(" (value:%s)", dv.c_str());
			}
			printf("\n\t\t%s\n", this->data[i].help_message.c_str());
		}
	}
	printf("\n");

	for (size_t i = 0; i < this->data.size(); i++)
	{
		if (this->data[i].number != -1)
		{
			printf("\t");
			String k = this->data[i].keys[0];
			k = k.substr(1, k.length() - 1);

			printf("%s", k.c_str());

			String dv = cat_string(this->data[i].def_value);
			if (dv.compare("") != 0)
			{
				printf(" (value:%s)", dv.c_str());
			}
			printf("\n\t\t%s\n", this->data[i].help_message.c_str());
		}
	}
}

std::vector<String> CommandLineParser::split_range_string(const String& _str, char fs, char ss) const
{
	String str = _str;
	std::vector<String> vec;
	String word = "";
	bool begin = false;
	while (!str.empty())
	{
		if (str[0] == fs)
		{
			if (begin == true)
			{
#if 0
				(cv::Exception(CV_StsParseError,
					String("error in split_range_string(")
					+ str
					+ String(", ")
					+ String(1, fs)
					+ String(", ")
					+ String(1, ss)
					+ String(")"),
					"", __FILE__, __LINE__
				));
#endif
			}
			begin = true;
			word = "";
			str = str.substr(1, str.length() - 1);
		}

		if (str[0] == ss)
		{
			if (begin == false)
			{
#if 0
				CV_THROW(cv::Exception(CV_StsParseError,
					String("error in split_range_string(")
					+ str
					+ String(", ")
					+ String(1, fs)
					+ String(", ")
					+ String(1, ss)
					+ String(")"),
					"", __FILE__, __LINE__
				));
#endif
			}
			begin = false;
			vec.push_back(word);
		}

		if (begin == true)
		{
			word = word + str[0];
		}
		str = str.substr(1, str.length() - 1);
	}

	if (begin == true)
	{
#if 0
		CV_THROW(cv::Exception(CV_StsParseError,
			String("error in split_range_string(")
			+ str
			+ String(", ")
			+ String(1, fs)
			+ String(", ")
			+ String(1, ss)
			+ String(")"),
			"", __FILE__, __LINE__
		));
#endif
	}
	return vec;
}

std::vector<String> CommandLineParser::split_string(const String& _str, char symbol, bool create_empty_item) const
{
	String str = _str;
	std::vector<String> vec;
	String word = "";

	while (!str.empty())
	{
		if (str[0] == symbol)
		{
			if (!word.empty() || create_empty_item)
			{
				vec.push_back(word);
				word = "";
			}
		}
		else
		{
			word = word + str[0];
		}
		str = str.substr(1, str.length() - 1);
	}

	if (word != "" || create_empty_item)
	{
		vec.push_back(word);
	}

	return vec;
}

#include <vector>
namespace gflags {

  struct flags_data {
    int id = 0;
    std::string __keys;
    std::vector<std::pair<String, String*>> v_string;
    std::vector<std::pair<String, int32_t*>> v_int32;
    std::vector<std::pair<String, bool*>> v_bool;
    int add(const char* name, String* pval, const char* val, const char* txt) {
      v_string.push_back(make_pair(String(name), pval));
      __keys += format("{%s | %s | %s}", name, val, txt);
      return id++;
    }
    int add(const char* name, int32_t* pval, int32_t val, const char* txt) {
      v_int32.push_back(make_pair(String(name), pval));
      __keys += format("{%s | %d | %s}", name, val, txt);
      return id++;
    }
    int add(const char* name, bool* pval, bool val, const char* txt) {
      v_bool.push_back(make_pair(String(name), pval));
      __keys += format("{%s | %d | %s}", name, val, txt);
      return id++;
    }

    int ParseCommandLineFlags(int argc, const char* const argv[], bool b) {
      CommandLineParser parser(argc, argv, __keys.c_str());
      for (auto s : v_string) {
        *(s.second) = parser.get<String>(s.first);
      }
      for (auto s : v_int32) {
        *(s.second) = parser.get<int32_t>(s.first);
      }
      for (auto s : v_bool) {
        *(s.second) = parser.get<bool>(s.first);
      }
      return 0;
    }

  } f_data;
#define DEFINE_string(name, val, txt)  std::string FLAGS_##name = val; int IDFLAGS_##name = gflags::f_data.add(#name, &FLAGS_##name, val, txt)
#define DEFINE_int32(name, val, txt)   int32_t FLAGS_##name = val; int IDFLAGS_##name = gflags::f_data.add(#name, &FLAGS_##name, val, txt)
#define DEFINE_bool(name, val, txt)    bool FLAGS_##name = val; int IDFLAGS_##name = gflags::f_data.add(#name, &FLAGS_##name, val, txt)

int ParseCommandLineFlags(int argc, const char* const argv[], bool b) {
  return f_data.ParseCommandLineFlags(argc, argv, b);
}

int SetVersionString(const char* version) {
  return 0;
}

int SetUsageMessage(const char* message) {
  return 0;
}

int ShowUsageWithFlagsRestrict(const char* usage, const char* flags_restrict) {
  return 0;
}

}


#endif // _COMMAND_LINE_PARSER_HPP_