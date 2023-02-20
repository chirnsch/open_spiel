// ToMarker
// ToCell
// GetBoard

#include "open_spiel/spiel.h"
#include "open_spiel/tests/basic_tests.h"

namespace open_spiel {
namespace secret_tic_tac_toe {
namespace {

namespace testing = open_spiel::testing;

void BasicTicTacToeTests() {
  testing::LoadGameTest("secret_tic_tac_toe");
  testing::NoChanceOutcomesTest(*LoadGame("secret_tic_tac_toe"));
  testing::RandomSimTest(*LoadGame("secret_tic_tac_toe"), 100);
}

}  // namespace
}  // namespace secret_tic_tac_toe
}  // namespace open_spiel

int main(int argc, char** argv) {
  // open_spiel::secret_tic_tac_toe::BasicTicTacToeTests();
}
