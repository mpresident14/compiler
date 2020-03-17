// Only need to specify default template arg (Void) once (in header file)
template <typename T>
template <typename Fn, typename Void>
Parser<T>::Parser(Fn&& f)
    : parseFn_(std::make_shared<std::unique_ptr<FnContainerAbstract>>(
          std::make_unique<FnContainer<Fn>>(std::forward<Fn>(f)))) {}

/*
 * When we set a parser recursively, it creates a cycle between a
 * shared_ptr and unique_ptr that we have to delete manually to
 * prevent a memory leak. We delete the function to which the unique_ptr
 * points if this is the last lazy parser referring to it.
 */
template <typename T>
Parser<T>::~Parser() {
  using namespace std;

  if (lazyCount_) {
    if (*lazyCount_ == 1) {
      FnContainerAbstract* fPtr = parseFn_->release();
      delete fPtr;
      delete lazyCount_;
    } else {
      --(*lazyCount_);
    }
  }
}

template <typename T>
Parser<T>::Parser(const Parser& other)
    : parseFn_(other.parseFn_), lazyCount_(other.lazyCount_) {
  if (lazyCount_) {
    ++(*lazyCount_);
  }
}

template <typename T>
Parser<T>::Parser(Parser&& other)
    : parseFn_(std::move(other.parseFn_)), lazyCount_(other.lazyCount_) {
  other.lazyCount_ = nullptr;
}

template <typename T>
T Parser<T>::parse(const std::string& input) const {
  using namespace std;

  size_t errPos = 0;
  stringstream inputStream(input);
  std::optional<T> optResult = (**parseFn_)(inputStream, &errPos);
  if (!optResult.has_value()) {
    string line;
    inputStream.seekg(errPos);
    getline(inputStream, line);
    throw invalid_argument("Parse error: Failed at: \"" + line + "\"");
  }

  if (inputStream.peek() != EOF) {
    string line;
    getline(inputStream, line);
    throw invalid_argument("Parser error: Characters remain at: \"" + line + "\"");
  }

  return move(optResult.value());
}

/*
 * andThen makes the guarantee that the parser in creates will reset the input stream
 * position if either parser fails. Combinators using andThen do not have to reset the
 * input stream
 */
template <typename T>
template <typename Fn, typename P>
P Parser<T>::andThen(Fn&& pGenFn) const {
  using namespace std;

  return {// This must be mutable to call pGenFn's non-const () operator
      [parseFn = isThisRValue() ? move(parseFn_) : parseFn_,
          pGenFn = forward<Fn>(pGenFn)](
          std::istream& input, size_t* errPos) mutable -> optional<parsers::ptype_t<P>> {
        // Always reset to original position if either parser fails
        size_t origPos = input.tellg();

        // Run first parser
        std::optional<T> optResult1 = (**parseFn)(input, errPos);
        // If first parser fails, reset the stream and fail entire thing
        if (!optResult1.has_value()) {
          input.seekg(origPos);
          *errPos = max(*errPos, origPos);
          return {};
        }

        // Run the next parser on the rest of the string
        auto nextParser = pGenFn(move(optResult1.value()));
        auto optResult2 = (**nextParser.parseFn_)(input, errPos);
        if (optResult2.has_value()) {
          return optResult2;
        }

        *errPos = max(*errPos, origPos);
        input.seekg(origPos);
        return optResult2;
      }};
}

template <typename T>
Parser<T> Parser<T>::alt(Parser<T> nextParser) const {
  using namespace std;

  return {[parseFn = isThisRValue() ? move(parseFn_) : parseFn_,
              parseFn2 = move(nextParser.parseFn_)](std::istream& input, size_t* errPos) {
    // Run first parser
    std::optional<T> optResult1 = (**parseFn)(input, errPos);
    // If first parser succeeds, return the result
    if (optResult1.has_value()) {
      return optResult1;
    }

    // Otherwise, try the next parser on the input
    std::optional<T> optResult2 = (**parseFn2)(input, errPos);
    if (optResult2.has_value()) {
      return optResult2;
    }
    return optResult2;
  }};
}

template <typename T>
template <typename Fn, typename R>
Parser<R> Parser<T>::transform(Fn&& mapFn) const {
  using namespace std;

  // Safe to move obj because andThen() will not need it again.
  return andThen([mapFn = forward<Fn>(mapFn)](
                     T&& obj) mutable { return parsers::createBasic(mapFn(move(obj))); });
}


// Pass nextParser by value since we have to copy it into the lambda anyways.
template <typename T>
template <typename R>
Parser<std::pair<T, R>> Parser<T>::combine(Parser<R> nextParser) const {
  using namespace std;

  return andThen([nextParser = move(nextParser)](T&& obj1) {
    return nextParser.andThen(
        // In order to move obj1, the lambda must be mutable
        // Safe to move obj1 and obj2 because of andThen() does not need their
        // values anymore.
        [obj1 = move(obj1)](R&& obj2) mutable {
          return parsers::createBasic(make_pair(move(obj1), move(obj2)));
        });
  });
}

template <typename T>
template <typename A, typename B>
Parser<std::tuple<T, A, B>> Parser<T>::combine(Parser<A> p1, Parser<B> p2) const {
  using namespace std;

  return andThen([p1 = move(p1), p2 = move(p2)](T&& obj) {
    return p1.andThen([obj = move(obj), p2 = move(p2)](A&& obj1) {
      return p2.andThen([obj = move(obj), obj1 = move(obj1)](B&& obj2) {
        return parsers::createBasic(make_tuple(move(obj), move(obj1), move(obj2)));
      });
    });
  });
}

template <typename T>
template <typename A, typename B, typename C>
Parser<std::tuple<T, A, B, C>> Parser<T>::combine(Parser<A> p1, Parser<B> p2, Parser<C> p3) const {
  using namespace std;

  return andThen([p1 = move(p1), p2 = move(p2), p3 = move(p3)](T&& obj) {
    return p1.andThen([obj = move(obj), p2 = move(p2), p3 = move(p3)](A&& obj1) {
      return p2.andThen([obj = move(obj), obj1 = move(obj1), p3 = move(p3)](B&& obj2) {
        return p3.andThen([obj = move(obj), obj1 = move(obj1), obj2 = move(obj2)](C&& obj3) {
          return parsers::createBasic(make_tuple(move(obj), move(obj1), move(obj2), move(obj3)));
        });
      });
    });
  });
}

template <typename T>
template <typename Fn, typename Void>
Parser<T> Parser<T>::verify(Fn&& boolFn) const {
  using namespace std;

  return andThen([boolFn = forward<Fn>(boolFn)](T&& obj) mutable -> Parser<T> {
    // Safe to move obj because andThen() will not need it again.
    if (!boolFn(move(obj))) {
      return parsers::fail<T>();
    }
    return parsers::createBasic(move(obj));
  });
}

template <typename T>
template <typename U>
Parser<std::enable_if_t<std::is_same_v<U, char>, std::string>> Parser<T>::many() const {
  using namespace std;

  return Parser<string>{[parseFn = isThisRValue() ? move(parseFn_) : parseFn_](
                            std::istream& input, size_t* errPos) {
    size_t oldErrPos = *errPos;
    // Run parser until it fails and put each result in the list
    string parsedChars;
    std::optional<char> optResult = (**parseFn)(input, errPos);
    while (optResult.has_value()) {
      // value() is a char, no need to move it
      parsedChars.append(1, optResult.value());
      optResult = (**parseFn)(input, errPos);
    }

    // Reset errPos because many() does not fail when the underlying parser fails.
    *errPos = oldErrPos;
    return parsers::createReturnObject(move(parsedChars));
  }};
}

template <typename T>
template <typename U>
Parser<std::enable_if_t<!std::is_same_v<U, char>, std::vector<T>>> Parser<T>::many()
    const {
  using namespace std;

  return Parser<vector<T>>{[parseFn = isThisRValue() ? move(parseFn_) : parseFn_](
                               std::istream& input, size_t* errPos) {
    size_t oldErrPos = *errPos;
    // Run parser until it fails and put each result in the list
    vector<T> parsedObjs;
    std::optional<T> optResult = (**parseFn)(input, errPos);
    while (optResult.has_value()) {
      parsedObjs.push_back(move(optResult.value()));
      optResult = (**parseFn)(input, errPos);
    }

    // Reset errPos because many() does not fail when the underlying parser fails
    *errPos = oldErrPos;
    return parsers::createReturnObject(move(parsedObjs));
  }};
}

template <typename T>
template <typename U>
Parser<std::enable_if_t<std::is_same_v<U, char>, std::string>> Parser<T>::some() const {
  using namespace std;

  return combine(many()).transform([](auto&& charAndString) {
    auto& str = get<1>(charAndString);
    str.insert(str.cbegin(), move(get<0>(charAndString)));
    return move(str);
  });
}

template <typename T>
template <typename U>
Parser<std::enable_if_t<!std::is_same_v<U, char>, std::vector<T>>> Parser<T>::some()
    const {
  using namespace std;

  return combine(many()).transform([](auto&& objAndObjVec) {
    auto& objVec = get<1>(objAndObjVec);
    objVec.insert(objVec.begin(), move(get<0>(objAndObjVec)));
    return move(objVec);
  });
}

template <typename T>
template <typename R>
Parser<R> Parser<T>::ignoreAndThen(Parser<R> nextParser) const {
  using namespace std;

  return andThen([nextParser = move(nextParser)](T&&) { return move(nextParser); });
}

template <typename T>
template <typename R>
Parser<T> Parser<T>::thenIgnore(Parser<R> nextParser) const {
  using namespace std;

  return andThen([nextParser = move(nextParser)](T&& obj) {
    return nextParser.transform(
        // In order to forward obj, the lambda must be mutable
        // Safe to move obj because andThen() will not need it again.
        [obj = move(obj)](R&&) mutable { return move(obj); });
  });
}

template <typename T>
void Parser<T>::set(Parser&& other) {
  if (lazyCount_) {
    throw std::runtime_error("Setting a parser that has already been set");
  }
  *parseFn_ = std::move(*other.parseFn_);
  lazyCount_ = new size_t(1);
}
