#include <iostream>
#include <optional>
#include <iomanip>
#include <variant>
#include <vector>
#include <memory>
#include <queue>

using namespace std;



class JsonBase
{
protected:
    JsonBase* parent_;
    queue<unique_ptr<JsonBase>> vals_;
    ostream& stream_;
    bool is_ended_ = false;

    JsonBase(ostream& stream, JsonBase* parent = nullptr) :
        parent_(parent),
        stream_(stream)
    {}


public:
    virtual ~JsonBase() = default;
    void PushValue(unique_ptr<JsonBase> val)
    {
        vals_.push(move(val));
    }
    ostream& GetStream()
    {
        return stream_;
    }
};

class JsonValue : public JsonBase, public variant<monostate, string_view, bool, int64_t>
{
public:
    JsonValue(ostream& stream, variant<monostate, string_view, bool, int64_t> val):
        JsonBase(stream),
        variant<monostate, string_view, bool, int64_t>(val)
    {}

    ~JsonValue() override
    {
        switch (this->index()) {
        case 0:
            stream_ << "null";
            break;
        case 1:
            stream_ << quoted(get<1>(*this));
            break;
        case 2:
            stream_ << boolalpha << get<2>(*this);
            break;
        case 3:
            stream_ << get<3>(*this);
            break;
        }
    }
};

template <typename T>
class JsonAdders
{
    auto& GetType()
    {
        return static_cast<T&>(*this);
    }

    auto& ReturnValue()
    {
        return GetType().ValueToReturn();
    }

public:
    auto& String(string_view str)
    {
        GetType().PushValue(make_unique<JsonValue>(GetType().GetStream(), str));
        return ReturnValue();
    }

    auto& Null()
    {
        GetType().PushValue(make_unique<JsonValue>(GetType().GetStream(), monostate()));
        return ReturnValue();
    }

    auto& Number(int64_t val)
    {
        GetType().PushValue(make_unique<JsonValue>(GetType().GetStream(), val));
        return ReturnValue();
    }

    auto& Boolean(bool val)
    {
        GetType().PushValue(make_unique<JsonValue>(GetType().GetStream(), val));
        return ReturnValue();
    }
};

template <typename T>
class JsonObject;

template <typename T>
class JsonArray: public JsonBase, public JsonAdders<JsonArray<T>>
{

public:
    JsonArray(ostream& stream, JsonBase* parent = nullptr) :
        JsonBase(stream, parent)
    {}

    JsonArray<T>& ValueToReturn()
    {
        return *this;
    }

    JsonArray<JsonArray<T>>& BeginArray()
    {
        vals_.push(make_unique<JsonArray<JsonArray<T>>>(stream_, this));
        return static_cast<JsonArray<JsonArray<T>>&>(*vals_.back());
    }

    JsonObject<JsonArray<T>>& BeginObject()
    {
        vals_.push(make_unique<JsonObject<JsonArray<T>>>(stream_, this));
        return static_cast<JsonObject<JsonArray<T>>&>(*vals_.back());
    }

    T& EndArray()
    {
        return static_cast<T&>(*parent_);
    }

    ~JsonArray() override
    {
        stream_ << "[";
        while(vals_.size() > 1)
        {
            vals_.pop();
            stream_ << ',';
        }
        if(vals_.size())
            vals_.pop();
        stream_ << "]";
    }
};

template <typename T>
class JsonKey: public JsonBase, public JsonAdders<JsonKey<T>>
{
    string_view name_;

public:
    JsonKey(ostream& stream, string_view name, JsonBase* parent = nullptr) :
        JsonBase(stream, parent),
        name_(name)
    {}

    T& ValueToReturn()
    {
        return static_cast<T&>(*parent_);
    }

    JsonArray<T>& BeginArray()
    {
        vals_.push(make_unique<JsonArray<T>>(stream_, parent_));
        return static_cast<JsonArray<T>&>(*vals_.back());
    }

    JsonObject<T> &BeginObject()
    {
        vals_.push(make_unique<JsonObject<T>>(stream_, parent_));
        return static_cast<JsonObject<T>&>(*vals_.back());
    }

    ~JsonKey() override
    {
        stream_ << quoted(name_) << ':';
        if(vals_.size())
            vals_.pop();
        else stream_ << "null";
    }
};

template <typename T>
class JsonObject: public JsonBase
{
public:
    JsonObject(ostream& stream, JsonBase* parent = nullptr) :
        JsonBase(stream, parent)
    {}

    JsonObject<T>& ValueToReturn()
    {
        return *this;
    }

    JsonKey<JsonObject<T>>& Key(string_view name)
    {
        vals_.push(make_unique<JsonKey<JsonObject<T>>>(this->stream_, name, this));
        return static_cast<JsonKey<JsonObject<T>>&>(*vals_.back());
    }

    T& EndObject()
    {
        return static_cast<T&>(*parent_);
    }

    ~JsonObject() override
    {
        stream_ << "{";
        while(vals_.size() > 1)
        {
            vals_.pop();
            stream_ << ',';
        }
        if(vals_.size())
            vals_.pop();
        stream_ << "}";
    }
};


void PrintJsonString(ostream& stream, string_view str)
{
    stream << quoted(str);
}

JsonArray<JsonBase> PrintJsonArray(ostream& stream)
{
    return JsonArray<JsonBase>(stream);
}

JsonObject<JsonBase> PrintJsonObject(ostream& stream)
{
    return JsonObject<JsonBase>(stream);
}
