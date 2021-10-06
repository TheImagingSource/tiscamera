# Copyright 2020 The Imaging Source Europe GmbH
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

from collections import deque
import time
import threading
import logging


log = logging.getLogger(__name__)


class TaskThread(threading.Thread):
    """
    Thread that executes a task every N seconds
    """

    def __init__(self, func, pars=None, interval=15.0):
        """

        """
        threading.Thread.__init__(self)
        self._finished = threading.Event()
        self.interval = interval
        self.user_func = func
        self.user_params = pars

    def set_interval(self, interval):
        """
        Set the number of seconds we sleep between executing our task
        """
        self.interval = interval

    def shutdown(self):
        """
        Stop this thread
        """
        self._finished.set()

    def start(self):
        """

        """
        self._finished.clear()
        super().start()

    def run(self):
        """
        Implicitly called by executing start()

        executes self.user_func every self.interval
        until self.shutdown() is called
        """

        while not self._finished.is_set():
            self.task()

            # sleep for interval or until shutdown
            self._finished.wait(self.interval)

    def task(self):
        """
        The task done by this thread - override in subclasses
        """
        if self.user_params:
            self.user_func(self.user_params)
        else:
            self.user_func()


class FPSCounter(object):
    """
    This class is intended for fps measurements

    Uses a delta from the frames of the last n seconds to calculate fps
    Also offers an average fps from start to now
    """

    def __init__(self):
        """
        Arguments:
        seconds: int continaing the amount of seconds that
                 shall be used for the fps calculation
        """
        self._running = False
        self._start_time = None
        self.framecounter = 0
        self.frames_in_last_second = 0
        self._actual_fps = 0.0
        self._delta_queue = deque()
        self.seconds_max = 5
        self._lock = threading.Lock()
        self._tick_thread = TaskThread(self.__update_values, None, 1.0)

    def start(self):
        """
        Start fps measurements
        """
        if self._running:
            raise RuntimeError("Already running")
        self._start_time = time.time()
        self.framecounter = 0
        self.frames_in_last_second = 0
        self._tick_thread._finished.clear()
        self._tick_thread.start()
        self._running = True

    def stop(self):
        """

        """
        self._tick_thread.shutdown()

        if self._running:
            self._running = False
            self._tick_thread.join()

    def tick(self):
        """
        Add frame
        """
        self.framecounter += 1

        with self._lock:
            self.frames_in_last_second += 1

    def __update_values(self):
        """
        will be called in a separate thread to keep fps values etc up to date.
        """

        frame_count = 0

        with self._lock:
            self._delta_queue.append(self.frames_in_last_second)
            self.frames_in_last_second = 0

            # we only want n entries. get rid of the rest
            while len(self._delta_queue) > self.seconds_max:
                self._delta_queue.popleft()

            seconds = len(self._delta_queue)
            for i in range(len(self._delta_queue)):
                frame_count += self._delta_queue[i]

        if seconds == 0:
            self._actual_fps = 0.0
        else:
            self._actual_fps = frame_count / seconds

    def get_fps(self):
        """
        return fps from the last n seconds
        """

        return self._actual_fps

    def get_avg_fps(self):
        """
        returns float containg the avg fps since start has been called
        """
        if self._start_time == 0:
            self._start_time = time.time()
            return 0.0
        else:
            diff = int(time.time() - self._start_time)
            if diff == 0:
                return 0.0
            actual_fps = self.framecounter / diff
            return actual_fps
