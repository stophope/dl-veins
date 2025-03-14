import os
import flwr
import numpy as np
from config import Config
from flwr.server.strategy.krum import Krum
from flwr.server.client_proxy import ClientProxy
from typing import List, Tuple, Union, Optional, Dict, Callable

from flwr.common import FitRes, Scalar, Parameters, MetricsAggregationFn, NDArrays


class VeremiKrum(Krum):

    # pylint: disable=too-many-arguments,too-many-instance-attributes
    def __init__(
            self,
            *,
            fraction_fit: float = 1.0,
            fraction_evaluate: float = 1.0,
            min_fit_clients: int = 2,
            min_evaluate_clients: int = 2,
            min_available_clients: int = 2, evaluate_fn: Optional[
                Callable[
                    [int, NDArrays, Dict[str, Scalar]],
                    Optional[Tuple[float, Dict[str, Scalar]]],
                ]
            ] = None,
            on_fit_config_fn: Optional[Callable[[int], Dict[str, Scalar]]] = None,
            on_evaluate_config_fn: Optional[Callable[[int], Dict[str, Scalar]]] = None,
            accept_failures: bool = True, initial_parameters: Optional[Parameters] = None,
            fit_metrics_aggregation_fn: Optional[MetricsAggregationFn] = None,
            evaluate_metrics_aggregation_fn: Optional[MetricsAggregationFn] = None,
            output_path: str = ""
    ) -> None:

        """Krum [Blanchard et al., 2017] strategy.

        Implementation based on https://arxiv.org/abs/1703.02757

        Parameters
        ----------
        fraction_fit : float, optional
            Fraction of clients used during training. Defaults to 1.0.
        fraction_evaluate : float, optional
            Fraction of clients used during validation. Defaults to 1.0.
        min_fit_clients : int, optional
            Minimum number of clients used during training. Defaults to 2.
        min_evaluate_clients : int, optional
            Minimum number of clients used during validation. Defaults to 2.
        min_available_clients : int, optional
            Minimum number of total clients in the system. Defaults to 2.
        num_malicious_clients : int, optional
            Number of malicious clients in the system. Defaults to 0.
        num_clients_to_keep : int, optional
            Number of clients to keep before averaging (MultiKrum). Defaults to 0, in
            that case classical Krum is applied.
        evaluate_fn : Optional[Callable[[int, NDArrays, Dict[str, Scalar]], Optional[Tuple[float, Dict[str, Scalar]]]]]
            Optional function used for validation. Defaults to None.
        on_fit_config_fn : Callable[[int], Dict[str, Scalar]], optional
            Function used to configure training. Defaults to None.
        on_evaluate_config_fn : Callable[[int], Dict[str, Scalar]], optional
            Function used to configure validation. Defaults to None.
        accept_failures : bool, optional
            Whether or not accept rounds containing failures. Defaults to True.
        initial_parameters : Parameters, optional
            Initial global model parameters.
        """

        super().__init__(fraction_fit=fraction_fit, fraction_evaluate=fraction_evaluate,
                         min_fit_clients=min_fit_clients, min_evaluate_clients=min_evaluate_clients,
                         min_available_clients=min_available_clients, evaluate_fn=evaluate_fn,
                         on_fit_config_fn=on_fit_config_fn, on_evaluate_config_fn=on_evaluate_config_fn,
                         accept_failures=accept_failures, initial_parameters=initial_parameters,
                         fit_metrics_aggregation_fn=fit_metrics_aggregation_fn,
                         evaluate_metrics_aggregation_fn=evaluate_metrics_aggregation_fn)

        self.output_path = output_path
        self.load_data()
        self.params = None

    def load_data(self):
        if self.initial_parameters is None:
            file = self.output_path + Config.weights_file
            if os.path.exists(file):
                npzfile = np.load(file)
                params = [npzfile[x] for x in npzfile]
                params = flwr.common.ndarrays_to_parameters(params)
                self.initial_parameters = params

    def aggregate_fit(
            self,
            server_round: int,
            results: List[Tuple[flwr.server.client_proxy.ClientProxy, flwr.common.FitRes]],
            failures: List[Union[Tuple[ClientProxy, FitRes], BaseException]],
    ) -> Tuple[Optional[Parameters], Dict[str, Scalar]]:
        aggregated_parameters, aggregated_metrics = super().aggregate_fit(server_round, results, failures)

        if aggregated_parameters is not None:
            # Convert `Parameters` to `List[np.ndarray]`
            aggregated_ndarrays: List[np.ndarray] = flwr.common.parameters_to_ndarrays(aggregated_parameters)
            self.params = aggregated_ndarrays

        return aggregated_parameters, aggregated_metrics

    def save_params(self):
        np.savez(f"{self.output_path}{Config.weights_file}", *self.params)