#pragma once

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>

namespace jno {

static const struct {
    char jno_dot = '.';
    char jno_obstacle = ',';
    char jno_space = ' ';
    char jno_nodePathBreaker = '/';
    char jno_commentLine[3] = "//";
    char jno_left_seperator = '\\';
    char jno_eof_segment = '\n';
    char jno_format_string = '\"';
    char jno_true_string[5] = "true";
    char jno_false_string[6] = "false";
    char jno_array_segments[2]{'{', '}'};
    char jno_trim_segments[5]{' ', '\t', '\n', '\r', '\v'};
} jno_syntax;

/// undefined type
typedef void* jvariant;
/// string type
typedef std::string jstring;
/// integer type
typedef std::int64_t jnumber;
/// real type
typedef double jreal;
/// logical type
typedef bool jbool;

enum JNOSerializedFormat { JNOBeautify, JNOCompact };

class jno_object_parser;

enum JNOType { Unknown = 0, JNOString = 1, JNOBoolean = 2, JNOReal = 3, JNONumber = 4 };

class jno_object_node {
    friend class jno_object_parser;
    std::string propertyName;
    std::uint8_t valueFlag;
    void* _bits = nullptr;
    mutable int* uses = nullptr;

    int decrementMemory();
    int incrementMemory();

#ifdef NDEBUG
    jno_object_node* prevNode = nullptr;
    jno_object_node* nextNode = nullptr;
#endif

   public:
    jno_object_node() = default;
    jno_object_node(const jno_object_node&);

    jno_object_node& operator=(const jno_object_node&);

    ~jno_object_node();

    inline bool isArray();
    inline bool isStruct();
    inline bool isValue();
    inline bool isNumber();
    inline bool isReal();
    inline bool isString();
    inline bool isBoolean();

    // Property name it is Node
    std::string& getPropertyName();

    [[deprecated]]void set_native_memory(void* memory);

    std::int64_t& toNumber();
    double& toReal();
    std::string& toString();
    bool& toBoolean();
    std::vector<std::int64_t>* toNumbers();
    std::vector<double>* toReals();
    std::vector<std::string>* toStrings();
    std::vector<bool>* toBooleans();
};

class jno_object_parser {
   public:
    using jstruct = std::map<int, jno_object_node>;

   private:
    void* _storage;
    jstruct entry;
    int avail(jstruct& entry, const char* source, int len, int levels = 0);

   public:
    jno_object_parser();
    jno_object_parser(const jno_object_parser&) = delete;
    virtual ~jno_object_parser();

    void deserialize(const char* filename);
    void deserialize(const std::string& source);
    void deserialize(const char* source, int len);
    void deserialize(const std::string& content, int depth = -1);
    std::string serialize();

    // Find node
    jno_object_node* at(const std::string& name);

    jstruct& get_struct();
    jstruct& get_struct(jno_object_node* node);

    // Find node from child
    // example, key="First/Second/Triple" -> Node
    // for has a node, HasNode
    jno_object_node* find_node(const std::string& nodePath);

    // search node by name or prefix value, example
    jno_object_node* search(const std::string& name);

    // result of occupied memory alls nodes
    jnumber occupied_memory();

    bool contains(const std::string& nodePath);
};

int jno_string_to_hash(const char* str, int len = INT32_MAX);

}  // namespace jno
