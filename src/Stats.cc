#include "Stats.h"

#include <algorithm>
#include <stdexcept>

// Stores the permanent combat stats.
BaseStats::BaseStats(int power, int ward): power{power}, ward{ward} {}

// Returns permanent Power.
int BaseStats::getPower() const {
    return power;
}

// Returns permanent Ward Rating.
int BaseStats::getWard() const {
    return ward;
}

// Stores ownership of the wrapped Stats object.
StatsDecorator::StatsDecorator(std::unique_ptr<Stats> wrapped): wrapped{std::move(wrapped)} {
    if (!this->wrapped) {
        throw std::invalid_argument{"StatsDecorator cannot wrap null stats"};
    }
}

// Forwards Power to the wrapped object.
int StatsDecorator::getPower() const {
    return wrapped->getPower();
}

// Forwards Ward Rating to the wrapped object.
int StatsDecorator::getWard() const {
    return wrapped->getWard();
}

// Adds the Surge Power wrapper.
SurgePowerStats::SurgePowerStats(std::unique_ptr<Stats> wrapped):
    StatsDecorator{std::move(wrapped)} {}

// Applies +5 Power.
int SurgePowerStats::getPower() const {
    return wrapped->getPower() + 5;
}

// Adds the Fortify Ward wrapper.
FortifyWardStats::FortifyWardStats(std::unique_ptr<Stats> wrapped):
    StatsDecorator{std::move(wrapped)} {}

// Applies +5 Ward Rating.
int FortifyWardStats::getWard() const {
    return wrapped->getWard() + 5;
}

// Adds the Sap Power wrapper.
SapPowerStats::SapPowerStats(std::unique_ptr<Stats> wrapped):
    StatsDecorator{std::move(wrapped)} {}

// Applies -5 Power with a floor at 0.
int SapPowerStats::getPower() const {
    return std::max(0, wrapped->getPower() - 5);
}

// Adds the Erode Ward wrapper.
ErodeWardStats::ErodeWardStats(std::unique_ptr<Stats> wrapped):
    StatsDecorator{std::move(wrapped)} {}

// Applies -5 Ward Rating with a floor at 0.
int ErodeWardStats::getWard() const {
    return std::max(0, wrapped->getWard() - 5);
}
