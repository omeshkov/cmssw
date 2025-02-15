#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string>

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "HLTrigger/HLTcore/interface/TriggerExpressionParser.h"

namespace {
  using namespace std::literals;

  bool testExpression(std::string const& expression, std::string const& expected = {}) {
    auto const* expr = triggerExpression::parse(expression);

    if (not expr) {
      edm::LogWarning("InvalidInput") << "Couldn't parse trigger-results expression \"" << expression << "\"";
      return false;
    }

    std::ostringstream out;
    expr->dump(out);
    std::string const& str = out.str();

    if (not expected.empty() and str != expected) {
      edm::LogWarning("InvalidInput")                         //
          << "Parsed expression: \"" << expression << "\"\n"  //
          << "as:                \"" << str << "\"\n"         //
          << "instead of:        \"" << expected << '"';
      return false;
    }

    edm::LogPrint("testExpression")                         //
        << "Parsed expression: \"" << expression << "\"\n"  //
        << "as:                \"" << str << '"';
    return true;
  }

}  // namespace

TEST_CASE("Test TriggerExpressionParser", "[TriggerExpressionParser]") {
  // examples of expressions supported by the triggerExpression parser
  SECTION("CorrectExpressions") {
    REQUIRE(testExpression("TRUE",  //
                           "TRUE"));
    REQUIRE(testExpression("FALSE",  //
                           "FALSE"));
    REQUIRE(testExpression("NOT (FALSE)",  //
                           "(NOT FALSE)"));
    REQUIRE(testExpression("(NOT FALSE) OR TRUE",  //
                           "((NOT FALSE) OR TRUE)"));
    REQUIRE(testExpression("NOTThisHLTPath AND TRUE AND NOT L1_A?_*",
                           "((Uninitialised_Path_Expression AND TRUE) AND (NOT Uninitialised_L1_Expression))"));
    REQUIRE(testExpression("NOT NOTThisHLTPath",  //
                           "(NOT Uninitialised_Path_Expression)"));
    REQUIRE(testExpression("ThisHLTANDNOTThatORTheOther",  //
                           "Uninitialised_Path_Expression"));
    REQUIRE(testExpression("TRUEPath AND NOTPath",  //
                           "(Uninitialised_Path_Expression AND Uninitialised_Path_Expression)"));
    REQUIRE(testExpression("NOT L1_SEED1 AND L1_SEED2*",  //
                           "((NOT Uninitialised_L1_Expression) AND Uninitialised_L1_Expression)"));
    REQUIRE(testExpression("NOT L1_SEED2 AND (HLT_PATH_? AND NOT HLT_PATH2_??_*)",  //
                           "((NOT Uninitialised_L1_Expression) AND (Uninitialised_Path_Expression AND (NOT "
                           "Uninitialised_Path_Expression)))"));
    REQUIRE(testExpression("NOT (HLT_Path1 AND HLT_Path2)",  //
                           "(NOT (Uninitialised_Path_Expression AND Uninitialised_Path_Expression))"));
    REQUIRE(testExpression("NOT (NOTHLT_Path OR HLT_Path2)",  //
                           "(NOT (Uninitialised_Path_Expression OR Uninitialised_Path_Expression))"));
    REQUIRE(testExpression(
        "((L1_A AND HLT_B) OR Dataset_C) AND NOT (Status_D OR Name_E OR HLT_F*) AND L1_??_?_?",  //
        "((((Uninitialised_L1_Expression AND Uninitialised_Path_Expression) OR Uninitialised_Path_Expression)"
        " AND (NOT ((Uninitialised_Path_Expression OR Uninitialised_Path_Expression)"
        " OR Uninitialised_Path_Expression))) AND Uninitialised_L1_Expression)"));
    REQUIRE(testExpression("NOT (NOT (HLT_Path1 AND HLT_Path_*))",  //
                           "(NOT (NOT (Uninitialised_Path_Expression AND Uninitialised_Path_Expression)))"));
    REQUIRE(testExpression("NOT NOT (HLT_Path1 AND L1_Seed_?? OR HLT_Path_*)",  //
                           "(NOT (NOT ((Uninitialised_Path_Expression AND Uninitialised_L1_Expression) OR "
                           "Uninitialised_Path_Expression)))"));
    REQUIRE(testExpression("NOT NOT HLT_Path1 AND L1_Seed_??",  //
                           "((NOT (NOT Uninitialised_Path_Expression)) AND Uninitialised_L1_Expression)"));
    REQUIRE(testExpression("NOT(THIS OR THAT)AND(L1_THEOTHER)OR(NOTFALSE)",  //
                           "(((NOT (Uninitialised_Path_Expression OR Uninitialised_Path_Expression))"
                           " AND Uninitialised_L1_Expression) OR Uninitialised_Path_Expression)"));
  }

  // examples of expressions not supported by the triggerExpression parser
  SECTION("IncorrectExpressions") {
    REQUIRE(not testExpression("A | B"));
    REQUIRE(not testExpression("A && B"));
    REQUIRE(not testExpression("NOT L1_SEED1 ANDD L1_SEED2*"));
    REQUIRE(not testExpression("NOT (NOTHLT_Path OR HLT_Path2))"));
    REQUIRE(not testExpression("HLT_Path* NOT TRUE"));
    REQUIRE(not testExpression("ThisPath ANDThatPath"));
    REQUIRE(not testExpression("ThisPath AND ThatPath AND OR"));
    REQUIRE(not testExpression("ThisPath AND ThatPath OR NOT"));
  }
}
