#include "open_spiel/games/secret_tic_tac_toe.h"

#include "spiel_globals.h"
#include "spiel_utils.h"

namespace open_spiel {
namespace secret_tic_tac_toe {
namespace {

// Facts about the game.
const GameType kGameType{
    /*short_name=*/"secret_tic_tac_toe",
    /*long_name=*/"Secret Tic Tac Toe",
    GameType::Dynamics::kSequential,
    GameType::ChanceMode::kDeterministic,
    GameType::Information::kImperfectInformation,
    GameType::Utility::kZeroSum,
    GameType::RewardModel::kTerminal,
    /*max_num_players=*/2,
    /*min_num_players=*/2,
    /*provides_information_state_string=*/true,
    /*provides_information_state_tensor=*/false,
    /*provides_observation_string=*/true,
    /*provides_observation_tensor=*/true,
    /*parameter_specification=*/{}  // no parameters
};

std::shared_ptr<const Game> Factory(const GameParameters& params) {
  return std::shared_ptr<const Game>(new SecretTicTacToeGame(params));
}

REGISTER_SPIEL_GAME(kGameType, Factory);

}  // namespace

int ToCell(Action action) {
  SPIEL_CHECK_GE(action, 0);
  SPIEL_CHECK_LE(action, kCells * 2 - 1);

  return action % kCells;
}

Marker ToMarker(Action action) {
  SPIEL_CHECK_GE(action, 0);
  SPIEL_CHECK_LE(action, kCells * 2 - 1);

  return static_cast<Marker>(action / kCells + 1);
}

std::string MarkerToString(Marker marker) {
  switch (marker) {
    case Marker::kEmpty:
      return ".";
    case Marker::kNought:
      return "o";
    case Marker::kCross:
      return "x";
    default:
      SpielFatalError("Unknown Marker.");
  }
}

int ToCellIndex(int row, int col) { return row * kSize + col; }

void PrintBoard(std::array<Marker, kCells> board) {
  std::cout << "Printing board...\n";
  for (int row = 0; row < kSize; row++) {
    for (int col = 0; col < kSize; col++) {
      std::cout << MarkerToString(board[ToCellIndex(row, col)]);
    }
    std::cout << "\n";
  }
}

std::array<Marker, kCells> GetBoard(
    const std::vector<State::PlayerAction>& history) {
  std::array<Marker, kCells> board = {Marker::kEmpty};
  for (State::PlayerAction player_action : history) {
    board[ToCell(player_action.action)] = ToMarker(player_action.action);
  }
  return board;
}

bool IsFull(const std::vector<State::PlayerAction>& history) {
  return history.size() == kCells;
}

// Assume that whoever made the last move is the winner, if any.
// Returns kInvalidPlayer if neither player won.
int GetWinner(const std::vector<State::PlayerAction>& history) {
  auto board = GetBoard(history);
  int equal_count;

  // Winner in any row.
  for (int row = 0; row < kSize; row++) {
    equal_count = 0;
    for (int col = 0; col < kSize; col++) {
      equal_count +=
          (board[ToCellIndex(row, col)] != Marker::kEmpty &&
           board[ToCellIndex(row, col)] == board[ToCellIndex(row, 0)]);
      if (equal_count == kSize) {
        return history.back().player;
      }
    }
  }

  // Winner in any column.
  for (int col = 0; col < kSize; col++) {
    equal_count = 0;
    for (int row = 0; row < kSize; row++) {
      equal_count +=
          (board[ToCellIndex(row, col)] != Marker::kEmpty &&
           board[ToCellIndex(row, col)] == board[ToCellIndex(0, col)]);
      if (equal_count == kSize) {
        return history.back().player;
      }
    }
  }

  // Winner in top-left to bottom-right diagonal.
  equal_count = 0;
  for (int i = 0; i < kSize; i++) {
    equal_count += (board[ToCellIndex(i, i)] != Marker::kEmpty &&
                    board[ToCellIndex(i, i)] == board[ToCellIndex(0, 0)]);
  }
  if (equal_count == kSize) {
    return history.back().player;
  }

  // Winner in top-right to bottom-left diagonal.
  equal_count = 0;
  for (int i = 0; i < kSize; i++) {
    equal_count += (board[ToCellIndex(i, kSize - i - 1)] != Marker::kEmpty &&
                    board[ToCellIndex(i, kSize - i - 1)] ==
                        board[ToCellIndex(0, kSize - 1)]);
  }
  if (equal_count == kSize) {
    return history.back().player;
  }

  return kInvalidPlayer;
}

std::string VisibleActionToString(State::PlayerAction player_action) {
  return std::to_string(ToCell(player_action.action)) +
         MarkerToString(ToMarker(player_action.action));
}

std::string HiddenActionToString(State::PlayerAction player_action) {
  return std::to_string(ToCell(player_action.action)) + "?";
}

std::string PlayerActionToString(State::PlayerAction player_action,
                                 Player player) {
  return player_action.player == player ? VisibleActionToString(player_action)
                                        : HiddenActionToString(player_action);
}

bool SecretTicTacToeState::IsTerminal() const {
  return IsFull(history_) || GetWinner(history_) != kInvalidPlayer;
}

std::vector<Action> SecretTicTacToeState::LegalActions() const {
  if (IsTerminal()) return {};

  auto board = GetBoard(history_);

  std::vector<Action> moves;
  for (int cell = 0; cell < kCells; cell++) {
    if (board[cell] == Marker::kEmpty) {
      // Either of the two symbols.
      moves.push_back(cell);
      moves.push_back(cell + kCells);
    }
  }
  return moves;
}

void SecretTicTacToeState::DoApplyAction(Action action) {
  SPIEL_CHECK_TRUE(GetBoard(history_)[ToCell(action)] == Marker::kEmpty);
  history_.push_back(PlayerAction{CurrentPlayer(), action});
}

void SecretTicTacToeState::UndoAction(Player player, Action move) {
  SPIEL_CHECK_GT(history_.size(), 0);
  history_.pop_back();
}

Player SecretTicTacToeState::CurrentPlayer() const {
  // Player 0 starts.
  return IsTerminal() ? kTerminalPlayerId : Player{history_.size() % 2};
}

std::vector<double> SecretTicTacToeState::Returns() const {
  int winner = GetWinner(history_);
  if (winner == Player{0}) {
    return {1, -1};
  } else if (winner == Player{1}) {
    return {-1, 1};
  } else {
    return {0, 0};
  }
}

std::string SecretTicTacToeState::ToString() const {
  std::vector<std::string> moves;
  for (PlayerAction player_action : history_) {
    moves.push_back(VisibleActionToString(player_action));
  }

  return absl::StrJoin(moves, ", ");
}
std::string SecretTicTacToeState::InformationStateString(Player player) const {
  SPIEL_CHECK_GE(player, 0);
  SPIEL_CHECK_LT(player, num_players_);

  std::vector<std::string> moves;
  for (PlayerAction player_action : history_) {
    moves.push_back(PlayerActionToString(player_action, player));
  }

  return absl::StrJoin(moves, ", ");
}

// Should not require information other than the players' actions:
// https://github.com/deepmind/open_spiel/blob/master/open_spiel/spiel.h#L124-L126
std::string SecretTicTacToeState::ObservationString(Player player) const {
  SPIEL_CHECK_GE(player, 0);
  SPIEL_CHECK_LT(player, num_players_);

  return "";
}

SecretTicTacToeState::SecretTicTacToeState(std::shared_ptr<const Game> game)
    : State(game) {
  history_.clear();
}

std::string SecretTicTacToeState::ActionToString(Player player,
                                                 Action action_id) const {
  return game_->ActionToString(player, action_id);
}

std::unique_ptr<State> SecretTicTacToeState::Clone() const {
  return std::unique_ptr<State>(new SecretTicTacToeState(*this));
}

SecretTicTacToeGame::SecretTicTacToeGame(const GameParameters& params)
    : Game(kGameType, params) {}

}  // namespace secret_tic_tac_toe
}  // namespace open_spiel