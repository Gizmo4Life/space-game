#include <algorithm>
#include <catch2/catch_test_macros.hpp>

// Simulated scrolling logic to verify clamping and offset behavior
struct ScrollState {
  int selectedIndex = 0;
  int scrollOffset = 0;
  int maxVisible = 10;
  int totalItems = 0;

  void update(int newIndex) {
    selectedIndex = std::clamp(newIndex, 0, std::max(0, totalItems - 1));

    if (selectedIndex < scrollOffset) {
      scrollOffset = selectedIndex;
    } else if (selectedIndex >= scrollOffset + maxVisible) {
      scrollOffset = selectedIndex - maxVisible + 1;
    }
  }
};

TEST_CASE("UI Scrolling Logic", "[ui][scrolling]") {
  ScrollState state;
  state.totalItems = 25;
  state.maxVisible = 10;

  SECTION("Initial state") {
    REQUIRE(state.selectedIndex == 0);
    REQUIRE(state.scrollOffset == 0);
  }

  SECTION("Navigate within visible range") {
    state.update(5);
    REQUIRE(state.selectedIndex == 5);
    REQUIRE(state.scrollOffset == 0);
  }

  SECTION("Scroll down past visible range") {
    state.update(10); // Index 10 is the 11th item
    REQUIRE(state.selectedIndex == 10);
    REQUIRE(state.scrollOffset == 1);
  }

  SECTION("Scroll to end") {
    state.update(24);
    REQUIRE(state.selectedIndex == 24);
    REQUIRE(state.scrollOffset == 15); // 24 - 10 + 1
  }

  SECTION("Scroll back up") {
    state.update(24);
    state.update(14);
    REQUIRE(state.scrollOffset == 14);
    state.update(5);
    REQUIRE(state.scrollOffset == 5);
  }

  SECTION("Clamping test") {
    state.update(-10);
    REQUIRE(state.selectedIndex == 0);
    state.update(100);
    REQUIRE(state.selectedIndex == 24);
  }
}
