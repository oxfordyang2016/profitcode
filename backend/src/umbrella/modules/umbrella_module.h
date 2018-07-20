#ifndef SRC_UMBRELLA_MODULES_UMBRELLA_MODULE_H_
#define SRC_UMBRELLA_MODULES_UMBRELLA_MODULE_H_

class Umbrella;

class UmbrellaModule {
 public:
  explicit UmbrellaModule(Umbrella* umbrella) : umbrella_(umbrella) {}
  virtual ~UmbrellaModule() {}
  virtual void Process() = 0;

  inline Umbrella* GetUmbrella() const { return umbrella_; }

 private:
  Umbrella* umbrella_;
};

#endif  // SRC_UMBRELLA_MODULES_UMBRELLA_MODULE_H_
