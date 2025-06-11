#ifndef ILEDFEATURE_H
#define ILEDFEATURE_H

class ILedFeature {
public:
    virtual ~ILedFeature() = default;

    virtual void tick() = 0;                  // 每次主定时器调用
    virtual void setEnabled(bool enabled) = 0;
    virtual bool isEnabled() const = 0;
};

#endif // ILEDFEATURE_H
