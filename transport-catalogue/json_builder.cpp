#include "json_builder.h"
#include "json.h"

using namespace std;

namespace json {

bool Builder::CheckIfReady() {
    if (started_ && total_arr_ == 0 && total_dict_ == 0) {
        return true;
    }
    return false;
}

Builder::Builder() {
    nodes_stack_.push_back(&root_);
}

json::Node& Builder::Build() {
    if (CheckIfReady()) {
        return root_;
    }
    throw std::logic_error("Object is unready");
}

DictItemContext& Builder::StartDict() {
    if (!CheckIfReady()) {
        started_ = true;

        total_dict_++;

        Dict dict;

        if (root_ == nullptr) {

            *nodes_stack_.back() = dict;
            DictItemContext* call_after_start_dict = static_cast<DictItemContext*>(new BaseContext(*this));
            return *call_after_start_dict;

        }

        Node* parent = nodes_stack_.back();

        if (parent->IsArray()) {
            std::get<Array>(*(parent->GetValueMine())).push_back(dict);
            nodes_stack_.push_back(&(std::get<Array>(*(parent->GetValueMine())).back()));
            DictItemContext* call_after_start_dict = static_cast<DictItemContext*>(new BaseContext(*this));
            return *call_after_start_dict;
        }

        if (parent->IsNull()) {
            *parent->GetValueMine() = dict;
            DictItemContext* call_after_start_dict = static_cast<DictItemContext*>(new BaseContext(*this));
            return *call_after_start_dict;
        }

        //return *this;
        throw std::logic_error("Bad StartDict() call");
    }
    throw std::logic_error("Object is ready, call Build()1");
}

KeyItemContext& Builder::Key(std::string key) {
    if (!CheckIfReady()) {
        if (holds_alternative<Dict>(*(nodes_stack_.back()->GetValueMine()))) {
            nodes_stack_.push_back(&(std::get<Dict>(*(nodes_stack_.back()->GetValueMine()))[key]));
            KeyItemContext* call_after_key = static_cast<KeyItemContext*>(new BaseContext(*this));
            return *call_after_key;
        }
        throw std::logic_error("Key() called outside the dict");
    }
    throw std::logic_error("Object is ready, call Build()2");

}

BaseContext& Builder::Value(json::Node value) {

    if (!CheckIfReady()) {


        if (root_ == nullptr) {

            started_ = true;

            root_ = value;

            BaseContext* base_context = new BaseContext(*this);
            return *base_context;
        }

        Node* parent = nodes_stack_.back();

        if (parent->IsNull()) {
            *(nodes_stack_.back()->GetValueMine()) = (*value.GetValueMine());
            nodes_stack_.pop_back();
            BaseContext* base_context = new BaseContext(*this);
            return *base_context;
        }

        if (parent->IsArray()) {
            std::get<Array>(*(nodes_stack_.back()->GetValueMine())).push_back(value);
            BaseContext* base_context = new BaseContext(*this);
            return *base_context;
        }

        //return *this;
        throw std::logic_error("Bad Value() call");
    }
    throw std::logic_error("Object is ready, call Build()3");
}

ArrItemContext& Builder::StartArray() {
    if (!CheckIfReady()) {
        started_ = true;

        total_arr_++;

        Array arr;

        if (root_ == nullptr) {
            root_ = arr;
            ArrItemContext* call_after_start_array = static_cast<ArrItemContext*>(new BaseContext(*this));
            return *call_after_start_array;
        }

        Node* parent = nodes_stack_.back();

        if (parent->IsArray()) {
            std::get<Array>(*(parent->GetValueMine())).push_back(arr);
            nodes_stack_.push_back(&(std::get<Array>(*(parent->GetValueMine())).back()));
            ArrItemContext* call_after_start_array = static_cast<ArrItemContext*>(new BaseContext(*this));
            return *call_after_start_array;
        }

        if (parent->IsNull()) {
            *parent->GetValueMine() = arr;
            ArrItemContext* call_after_start_array = static_cast<ArrItemContext*>(new BaseContext(*this));
            return *call_after_start_array;
        }

        //return *this;
        throw std::logic_error("Bad StartArray() call");

    }
    throw std::logic_error("Object is ready, call Build()");
}

BaseContext& Builder::EndDict() {
    if (!CheckIfReady()) {
        if (std::holds_alternative<Dict>(*nodes_stack_.back()->GetValueMine())) {
            total_dict_--;
            nodes_stack_.pop_back();
            BaseContext* base_context = new BaseContext(*this);
            return *base_context;
        }
        throw std::logic_error("EndDict() in bad contex");
    }
    throw std::logic_error("Object is ready, call Build()");

}

BaseContext& Builder::EndArray() {
    if (!CheckIfReady()) {
        if ( std::holds_alternative<Array>(*nodes_stack_.back()->GetValueMine()) ) {
            total_arr_--;
            nodes_stack_.pop_back();
            BaseContext* base_context = new BaseContext(*this);
            return *base_context;
        }
        throw std::logic_error("EndArray() in bad contex");
    }
    throw std::logic_error("Object is ready, call Build()");

}

BaseContext::BaseContext(Builder& builder) : builder_(builder) {

}

BaseContext& BaseContext::Value(json::Node value) {
    return builder_.Value(value);
}

json::Node& BaseContext::Build() {
    return builder_.Build();
}

BaseContext& BaseContext::EndDict() {
    if (!builder_.CheckIfReady()) {
        if (std::holds_alternative<Dict>(*builder_.nodes_stack_.back()->GetValueMine())) {
            builder_.total_dict_--;
            builder_.nodes_stack_.pop_back();
            BaseContext* base_context = new BaseContext(*this);
            return *base_context;
        }
        throw std::logic_error("EndDict() in bad contex");
    }
    throw std::logic_error("Object is ready, call Build()");

}

BaseContext& BaseContext::EndArray() {
    if (!builder_.CheckIfReady()) {
        if ( std::holds_alternative<Array>(*builder_.nodes_stack_.back()->GetValueMine()) ) {
            builder_.total_arr_--;
            builder_.nodes_stack_.pop_back();
            BaseContext* base_context = new BaseContext(*this);
            return *base_context;
        }
        throw std::logic_error("EndArray() in bad contex");
    }
    throw std::logic_error("Object is ready, call Build()");

}

BaseContext& BaseContext::StartArray() {
    if (!builder_.CheckIfReady()) {
        builder_.started_ = true;

        builder_.total_arr_++;

        Array arr;

        if (builder_.root_ == nullptr) {
            builder_.root_ = arr;
            BaseContext* base_context = new BaseContext(*this);
            return *base_context;
        }

        Node* parent = builder_.nodes_stack_.back();

        if (parent->IsArray()) {
            std::get<Array>(*(parent->GetValueMine())).push_back(arr);
            builder_.nodes_stack_.push_back(&(std::get<Array>(*(parent->GetValueMine())).back()));
            BaseContext* base_context = new BaseContext(*this);
            return *base_context;
        }

        if (parent->IsNull()) {
            *parent->GetValueMine() = arr;
            BaseContext* base_context = new BaseContext(*this);
            return *base_context;
        }

        //return *this;
        throw std::logic_error("Bad StartArray() call");

    }
    throw std::logic_error("Object is ready, call Build()");
}

BaseContext& BaseContext::StartDict() {
    if (!builder_.CheckIfReady()) {
        builder_.started_ = true;

        builder_.total_dict_++;

        Dict dict;

        if (builder_.root_ == nullptr) {

            *(builder_.nodes_stack_.back()) = dict;
            BaseContext* base_context = new BaseContext(*this);
            return *base_context;

        }

        Node* parent = builder_.nodes_stack_.back();

        if (parent->IsArray()) {
            std::get<Array>(*(parent->GetValueMine())).push_back(dict);
            builder_.nodes_stack_.push_back(&(std::get<Array>(*(parent->GetValueMine())).back()));
            BaseContext* base_context = new BaseContext(*this);
            return *base_context;
        }

        if (parent->IsNull()) {
            *parent->GetValueMine() = dict;
            BaseContext* base_context = new BaseContext(*this);
            return *base_context;
        }

        //return *this;
        throw std::logic_error("Bad StartDict() call");
    }
    throw std::logic_error("Object is ready, call Build()1");
}

BaseContext& BaseContext::Key(std::string key) {
    if (!builder_.CheckIfReady()) {
        if (holds_alternative<Dict>(*(builder_.nodes_stack_.back()->GetValueMine()))) {
            builder_.nodes_stack_.push_back(&(std::get<Dict>(*(builder_.nodes_stack_.back()->GetValueMine()))[key]));
            BaseContext* base_context = new BaseContext(*this);
            return *base_context;
        }
        throw std::logic_error("Key() called outside the dict");
    }
    throw std::logic_error("Object is ready, call Build()2");

}

KeyItemContext& ValueKeyItemContext::Key(std::string key) {
    return builder_.Builder::Key(key);
}

BaseContext& ValueKeyItemContext::EndDict() {
    return builder_.Builder::EndDict();
}

BaseContext& ValueArrItemContext::EndArray() {
    return builder_.Builder::EndArray();

}

ArrItemContext& ValueArrItemContext::StartArray() {
    return builder_.Builder::StartArray();
}

DictItemContext& ValueArrItemContext::StartDict() {
    return builder_.Builder::StartDict();
}

ValueArrItemContext& ValueArrItemContext::Value(json::Node value) {
    return static_cast<ValueArrItemContext&>(builder_.Value(value));
}

KeyItemContext& DictItemContext::Key(std::string key) {
    return builder_.Builder::Key(key);
}

BaseContext& DictItemContext::EndDict() {
    return builder_.Builder::EndDict();
}

DictItemContext& KeyItemContext::StartDict() {
    return builder_.Builder::StartDict();
}

ArrItemContext& KeyItemContext::StartArray() {
    return builder_.Builder::StartArray();
}

ValueKeyItemContext& KeyItemContext::Value(json::Node value) {
    return static_cast<ValueKeyItemContext&>(builder_.Value(value));
}

BaseContext& ArrItemContext::EndArray() {
    return builder_.Builder::EndArray();
}

ArrItemContext& ArrItemContext::StartArray() {
    return builder_.Builder::StartArray();
}

DictItemContext& ArrItemContext::StartDict() {
    return builder_.Builder::StartDict();
}

ValueArrItemContext& ArrItemContext::Value(json::Node value) {
    return static_cast<ValueArrItemContext&>(builder_.Value(value));
}

}
