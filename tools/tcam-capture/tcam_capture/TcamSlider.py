# Copyright 2019 The Imaging Source Europe GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from PyQt5.QtWidgets import QSlider
from PyQt5.QtCore import pyqtSignal, Qt

import math
import logging

log = logging.getLogger(__name__)


class TcamSlider(QSlider):
    """
    A default QSlider does not offer double click.
    We want this to allow certain action.
    """

    doubleClicked = pyqtSignal()

    def __init__(self, parent=None):
        super(TcamSlider, self).__init__(Qt.Horizontal, parent)

    def mouseDoubleClickEvent(self, event):

        self.doubleClicked.emit()

        super().mouseDoubleClickEvent(event)


def log_(value):
    """
    Wrapper around math.log
    """

    if value == 0:
        return 0
    else:
        return math.log(value)


class TcamLogSlider(TcamSlider):
    """
    Logarithmic slider

    !!! Do not use valueChanged signal !!!
    Use valueLogChanged instead.
    Overwriting signals causes problems,
    thus is avoided.
    """
    valueLogChanged = pyqtSignal(int)

    def __init__(self, parent=None):
        super(TcamLogSlider, self).__init__(parent)

        self.minval = 0
        self.maxval = 100

        self.prop_min = 0
        self.prop_max = 100
        self.prop_step = 1

        self.log_slider_ticks = 100

        super(TcamLogSlider, self).valueChanged[int].connect(self._value_changed)

    def calc_log_val(self, value):
        """
        Calculate value for logarithmic slider
        """

        min_val = log_(self.prop_min)

        try:
            rangelen = math.log(self.prop_max) - min_val
        except ValueError as e:
            log.error("Rangelength error {}. {} - {} does not make sense".format(e, self.prop_max, self.prop_min))
            rangelen = 100

        val_val = log_(value)

        val = (self.log_slider_ticks / rangelen
               * (val_val - min_val))

        return val

    def calc_norm_val(self, value):
        """
        Returns:
        int/double:
        """
        minval = log_(self.prop_min)

        rangelen = math.log(self.prop_max) - minval

        val = math.exp(minval + rangelen
                       / self.log_slider_ticks * value)

        if val < minval:
            val = minval
        if val > self.prop_max:
            val = self.prop_max

        return val

    def setValue(self, value):
        """

        """

        super().setValue(self.calc_log_val(value))

    def setRange(self, min_val, max_val):

        self.prop_min = min_val
        self.prop_max = max_val
        super().setRange(self.minval, self.maxval)

    def setSingleStep(self, step):

        self.prop_step = 1
        super().setSingleStep(1)

    def _value_changed(self, val):
        """"""
        value = int(self.calc_norm_val(val))
        self.valueLogChanged.emit(value)
