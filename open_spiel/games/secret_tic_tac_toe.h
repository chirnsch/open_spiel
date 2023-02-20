#ifndef OPEN_SPIEL_GAMES_SECRET_TIC_TAC_TOE_H_
#define OPEN_SPIEL_GAMES_SECRET_TIC_TAC_TOE_H_

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "open_spiel/spiel.h"

namespace open_spiel {
namespace secret_tic_tac_toe {

inline constexpr int kNumPlayers = 2;
inline constexpr int kSize = 3;
inline constexpr int kCells = kSize * kSize;

enum class Marker {
  kEmpty,
  kNought,  // O
  kCross,   // X
};

class SecretTicTacToeState : public State {
 public:
  SecretTicTacToeState(std::shared_ptr<const Game> game);

  SecretTicTacToeState(const SecretTicTacToeState&) = default;
  SecretTicTacToeState& operator=(const SecretTicTacToeState&) = default;

  bool IsTerminal() const override;
  Player CurrentPlayer() const override;
  std::vector<Action> LegalActions() const override;
  void UndoAction(Player player, Action move) override;
  std::vector<double> Returns() const override;

  std::string ToString() const override;
  std::string InformationStateString(Player player) const override;
  std::string ObservationString(Player player) const override;

  // InformationStateTensor uses float values which is useful
  // "reinforcement learning and neural networks".
  // void InformationStateTensor(Player player,
  //                             absl::Span<float> values) const override;
  // void ObservationTensor(Player player,
  //                       absl::Span<float> values) const override;

  // Required to be overridden.
  std::string ActionToString(Player player, Action action_id) const override;
  std::unique_ptr<State> Clone() const override;

  // Required in tests.
  void ObservationTensor(Player player,
                         absl::Span<float> values) const override {
    return;
  }

 protected:
  void DoApplyAction(Action action_id) override;
  std::vector<PlayerAction> history_;
};

class SecretTicTacToeGame : public Game {
 public:
  explicit SecretTicTacToeGame(const GameParameters& params);

  std::unique_ptr<State> NewInitialState() const override {
    return std::unique_ptr<State>(new SecretTicTacToeState(shared_from_this()));
  }
  int NumPlayers() const override { return kNumPlayers; }
  int NumDistinctActions() const override { return kCells; }
  double MinUtility() const override { return -1; }
  double UtilitySum() const override { return 0; }
  double MaxUtility() const override { return 1; }
  int MaxGameLength() const override { return kCells; }

  // Required in tests.
  std::vector<int> ObservationTensorShape() const override { return {0}; }
};

}  // namespace secret_tic_tac_toe
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_SECRET_TIC_TAC_TOE_H_
