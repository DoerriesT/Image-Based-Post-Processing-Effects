#pragma once
#include <cstdint>

class BaseSystem
{
public:
	virtual ~BaseSystem() = default;
	virtual void init() = 0;
	virtual void input(const double &_currentTime, const double &_timeDelta) = 0;
	virtual void update(const double &_currentTime, const double &_timeDelta) = 0;
	virtual void render() = 0;
	virtual std::uint64_t getTypeIdOfDerived() = 0;

protected:
	static std::uint64_t typeCount;
};

template<typename Type>
class System : public BaseSystem
{
public:
	typedef Type SystemType;

	static std::uint64_t getTypeId();
	std::uint64_t getTypeIdOfDerived() override { return getTypeId(); }
};

template<typename Type>
inline std::uint64_t System<Type>::getTypeId()
{
	static const std::uint64_t ONE = 1;
	static const std::uint64_t type = ONE << typeCount++;
	return type;
}