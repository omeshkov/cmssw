#include <cassert>
#include <sstream>

#include "FWCore/Utilities/interface/RegexMatch.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "HLTrigger/HLTcore/interface/TriggerExpressionPathReader.h"
#include "HLTrigger/HLTcore/interface/TriggerExpressionData.h"

namespace triggerExpression {

  // define the result of the module from the HLT reults
  bool PathReader::operator()(const Data& data) const {
    if (not data.hasHLT() && not data.usePathStatus())
      return false;

    for (auto const& trigger : m_triggers)
      if (data.passHLT(trigger.second))
        return true;

    return false;
  }

  void PathReader::dump(std::ostream& out) const {
    if (not m_initialised) {
      out << "Uninitialised_Path_Expression";
      return;
    }
    if (m_triggers.empty()) {
      out << "FALSE";
    } else if (m_triggers.size() == 1) {
      out << m_triggers[0].first;
    } else {
      out << "(" << m_triggers[0].first;
      for (unsigned int i = 1; i < m_triggers.size(); ++i)
        out << " OR " << m_triggers[i].first;
      out << ")";
    }
  }

  // (re)initialize the module
  void PathReader::init(const Data& data) {
    // clear the previous configuration
    m_triggers.clear();

    // check if the pattern has is a glob expression, or a single trigger name
    if (not edm::is_glob(m_pattern)) {
      // no wildcard expression
      auto index = data.triggerIndex(m_pattern);
      if (index >= 0)
        m_triggers.push_back(std::make_pair(m_pattern, index));
      else {
        std::stringstream msg;
        msg << "requested HLT path \"" << m_pattern << "\" does not exist - known paths are:";
        if (data.triggerNames().empty())
          msg << " (none)";
        else
          for (auto const& p : data.triggerNames())
            msg << "\n\t" << p;
        if (data.shouldThrow())
          throw cms::Exception("Configuration") << msg.str();
        else
          edm::LogWarning("Configuration") << msg.str();
      }
    } else {
      // expand wildcards in the pattern
      const std::vector<std::vector<std::string>::const_iterator>& matches =
          edm::regexMatch(data.triggerNames(), m_pattern);
      if (matches.empty()) {
        // m_pattern does not match any trigger paths
        std::stringstream msg;
        msg << "requested pattern \"" << m_pattern << "\" does not match any HLT paths - known paths are:";
        if (data.triggerNames().empty())
          msg << " (none)";
        else
          for (auto const& p : data.triggerNames())
            msg << "\n\t" << p;
        if (data.shouldThrow())
          throw cms::Exception("Configuration") << msg.str();
        else
          edm::LogWarning("Configuration") << msg.str();
      } else {
        // store indices corresponding to the matching triggers
        for (auto const& match : matches) {
          auto index = data.triggerIndex(*match);
          assert(index >= 0);
          m_triggers.push_back(std::make_pair(*match, index));
        }
      }
    }

    m_initialised = true;
  }

}  // namespace triggerExpression
