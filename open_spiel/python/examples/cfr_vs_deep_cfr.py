from absl import app
from keras.models import load_model
from absl import flags
import tensorflow as tf
import numpy as np
import abc
import pyspiel
from typing import Sequence

FLAGS = flags.FLAGS
flags.DEFINE_string("filepath_cfr", "output.txt", "Filepath of the CFR policy")
flags.DEFINE_string("filepath_deep_cfr", "deep_cfr.txt",
                    "Filepath of the deep CFR policy")

# TODO: Add a thin wrapper that returns actions


class Bot(abc.ABC):

    @abc.abstractmethod
    def get_policy(self, information_state_string: str, information_state_tensor: Sequence[float], legal_actions: Sequence[int]) -> np.ndarray:
        raise NotImplementedError

    def get_action(self, policy: np.ndarray, legal_actions: Sequence[int]) -> int:
        return np.random.choice(legal_actions, p=policy)


class CfrBot(Bot):

    def __init__(self):
        f = open(FLAGS.filepath_cfr, "r")
        file = f.read()
        entries = iter(file.split('\n')[-1].split('<~>'))
        self.policy = {}
        for entry in entries:
            # legal_actions, cumulative_regrets, cumulative_policy, current_policy
            self.policy[entry] = [float.fromhex(val) for val in next(
                entries).split(';')[2].split(',')]

    def get_policy(self, information_state_string: str, information_state_tensor: Sequence[float], legal_actions: Sequence[int]) -> np.ndarray:
        del information_state_tensor

        policy = np.array(self.policy[information_state_string])
        assert len(policy) == len(legal_actions)
        return policy / sum(policy)


class RandomBot(Bot):

    def get_policy(self, information_state_string: str, information_state_tensor: Sequence[float], legal_actions: Sequence[int]) -> np.ndarray:
        return np.ones(len(legal_actions)) / len(legal_actions)


class DeepCfrBot(Bot):

    def __init__(self):
        self.model = load_model(FLAGS.filepath_deep_cfr)

    def get_policy(self, information_state_string: str, information_state_tensor: Sequence[float], legal_actions: Sequence[int]) -> np.ndarray:
        del information_state_string

        mask = tf.sparse.to_dense(
            tf.sparse.SparseTensor(
                dense_shape=[18],
                values=[1] * len(legal_actions),
                indices=[[idx] for idx in sorted(legal_actions)])
        )
        return self.model.call(
            ([information_state_tensor], [mask]))[0].numpy()[legal_actions]


def play_game(player_dict):
    game = pyspiel.load_game('secret_tic_tac_toe')
    state = game.new_initial_state()
    while not state.is_terminal():
        player = player_dict[state.current_player()]

        policy = player.get_policy(state.information_state_string(),
                                   state.information_state_tensor(), state.legal_actions())
        action = player.get_action(policy, state.legal_actions())
        state.apply_action(action)

    return state.returns()


def main(unused_argv):
    deep_cfr_bot = DeepCfrBot()
    cfr_bot = RandomBot()

    result = []
    for _ in range(200):
        returns = play_game({0: deep_cfr_bot, 1: cfr_bot})
        result.append(int(returns[0] > 0))

    print(sum(result) / len(result))

    result = []
    for _ in range(200):
        returns = play_game({0: cfr_bot, 1: deep_cfr_bot})
        result.append(int(returns[0] > 0))

    print(sum(result) / len(result))


if __name__ == "__main__":
    app.run(main)
