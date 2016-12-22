#ifndef OSRM_ENGINE_GUIDANCE_COLLAPSE_SCENARIO_DETECTION_HPP_
#define OSRM_ENGINE_GUIDANCE_COLLAPSE_SCENARIO_DETECTION_HPP_

#include "engine/guidance/collapsing_utility.hpp"
#include "engine/guidance/route_step.hpp"

namespace osrm
{
namespace engine
{
namespace guidance
{

// Staggered intersection are very short zig-zags of a few meters.
// We do not want to announce these short left-rights or right-lefts:
//
//      * -> b      a -> *
//      |       or       |       becomes  a   ->   b
// a -> *                * -> b
//
bool isStaggeredIntersection(const RouteStepIterator step_prior_to_intersection,
                             const RouteStepIterator step_entering_intersection,
                             const RouteStepIterator step_leaving_intersection);

// Two two turns following close after another, we can announce them as a U-Turn if both end up
// involving the same (segregated) road.
//
// b < - y
//       |      will be represented by at x, turn around instead of turn left at x, turn left at y
// a - > x
bool isUTurn(const RouteStepIterator step_prior_to_intersection,
             const RouteStepIterator step_entering_intersection,
             const RouteStepIterator step_leaving_intersection);

} /* namespace guidance */
} /* namespace engine */
} /* namespace osrm */

#endif /* OSRM_ENGINE_GUIDANCE_COLLAPSE_SCENARIO_DETECTION_HPP_ */