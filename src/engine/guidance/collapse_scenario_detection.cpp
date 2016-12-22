#include "engine/guidance/collapse_scenario_detection.hpp"
#include "util/bearing.hpp"

#include <boost/assert.hpp>

namespace osrm
{
namespace engine
{
namespace guidance
{

bool isStaggeredIntersection(const RouteStepIterator step_prior_to_intersection,
                             const RouteStepIterator step_entering_intersection,
                             const RouteStepIterator step_leaving_intersection)
{
    BOOST_ASSERT(!hasWaypointType(*step_entering_intersection) &&
                 !(hasWaypointType(*step_leaving_intersection)));
    // don't touch roundabouts
    if (entersRoundabout(step_entering_intersection->maneuver.instruction) ||
        entersRoundabout(step_leaving_intersection->maneuver.instruction))
        return false;
    // Base decision on distance since the zig-zag is a visual clue.
    // If adjusted, make sure to check validity of the is_right/is_left classification below
    const constexpr auto MAX_STAGGERED_DISTANCE = 3; // debatable, but keep short to be on safe side

    const auto angle = [](const RouteStep &step) {
        const auto &intersection = step.intersections.front();
        const auto entry_bearing = intersection.bearings[intersection.in];
        const auto exit_bearing = intersection.bearings[intersection.out];
        return util::angleBetweenBearings(entry_bearing, exit_bearing);
    };

    // Instead of using turn modifiers (e.g. as in isRightTurn) we want to be more strict here.
    // We do not want to trigger e.g. on sharp uturn'ish turns or going straight "turns".
    // Therefore we use the turn angle to derive 90 degree'ish right / left turns.
    // This more closely resembles what we understand as Staggered Intersection.
    // We have to be careful in cases with larger MAX_STAGGERED_DISTANCE values. If the distance
    // gets large, sharper angles might be not obvious enough to consider them a staggered
    // intersection. We might need to consider making the decision here dependent on the actual turn
    // angle taken. To do so, we could scale the angle-limits by a factor depending on the distance
    // between the turns.
    const auto is_right = [](const double angle) { return angle > 45 && angle < 135; };
    const auto is_left = [](const double angle) { return angle > 225 && angle < 315; };

    const auto left_right =
        is_left(angle(*step_entering_intersection)) && is_right(angle(*step_leaving_intersection));
    const auto right_left =
        is_right(angle(*step_entering_intersection)) && is_left(angle(*step_leaving_intersection));

    // A RouteStep holds distance/duration from the maneuver to the subsequent step.
    // We are only interested in the distance between the first and the second.
    const auto is_short = step_entering_intersection->distance < MAX_STAGGERED_DISTANCE;
    const auto intermediary_mode_change =
        step_prior_to_intersection->mode == step_leaving_intersection->mode &&
        step_entering_intersection->mode != step_leaving_intersection->mode;

    const auto mode_change_when_entering =
        step_prior_to_intersection->mode != step_entering_intersection->mode;

    // previous step maneuver intersections should be length 1 to indicate that
    // there are no intersections between the two potentially collapsible turns
    const auto no_intermediary_intersections =
        step_entering_intersection->intersections.size() == 1;

    return is_short && (left_right || right_left) && !intermediary_mode_change &&
           !mode_change_when_entering && no_intermediary_intersections;
}

bool isUTurn(const RouteStepIterator step_prior_to_intersection,
             const RouteStepIterator step_entering_intersection,
             const RouteStepIterator step_leaving_intersection)
{
    // require modes to match up
    if (!haveSameMode(*step_prior_to_intersection, *step_entering_intersection) ||
        !haveSameMode(*step_entering_intersection, *step_leaving_intersection))
        return false;

    const bool takes_u_turn = bearingsAreReversed(
        util::reverseBearing(step_entering_intersection->intersections.front()
                                 .bearings[step_entering_intersection->intersections.front().in]),
        out_step.intersections.front().bearings[out_step.intersections.front().out]);

    if (!takes_u_turn)
        return false;

    // TODO check for name match after additional step
    const auto names_match = haveSameName(step_prior_to_intersection, step_leaving_intersection);

    if (!names_match)
        return false;

    const auto is_short = step_entering_intersection->distance <= MAX_COLLAPSE_DISTANCE;
    const auto only_allowed_turn = numberOfAllowedTurns(step_leaving_intersection) == 1;
    return is_short || isLinkroad(*step_entering_intersection) || only_allowed_turn;
}

} /* namespace guidance */
} /* namespace engine */
} /* namespace osrm */