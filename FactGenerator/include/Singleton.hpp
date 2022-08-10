#ifndef SINGLETON_HPP__
#define SINGLETON_HPP__

template <typename T>
class Singleton {
 protected:
  Singleton() = default;

  static T* INSTANCE;

  virtual ~Singleton() = default;

 public:
  static auto getInstance() -> T* {
    if (!INSTANCE) INSTANCE = new T();
    return INSTANCE;
  }

  static void destroy() {
    delete INSTANCE;
    INSTANCE = 0;
  }

  auto isInitialized() -> bool { return INSTANCE != 0; }
};

#endif
