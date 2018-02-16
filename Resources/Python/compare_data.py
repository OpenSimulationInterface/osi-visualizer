#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
#from __future__ import unicode_literals

import argparse
import sys
import os
import logging

from termcolor import colored
from numpy import sort


class Comparision(object):
    """
    Base class for comparisons performed between the two files

    This class stores following fields as class fields

     -* data_file_a    - name (including path) of main file
     -* data_file_b    - name (including path) of benchmarked file
     -* read_message_limit - limit on number of messages read from data file

    """
    epsilon = 10. ** -8.
    absolute_total = 0.
    data_file_a = ''
    data_file_b = ''
    read_message_limit = None

    @classmethod
    def get_filename_a(cls):
        return os.path.basename(cls.data_file_a)

    @classmethod
    def get_filename_b(cls):
        return os.path.basename(cls.data_file_b)

    @classmethod
    def are_floats_equal(cls, a, b):
        return abs(a - b) <= max(cls.epsilon * max(abs(a), abs(b)), cls.absolute_total)


class CompareData(Comparision):
    """
    Compare two data osi files with multiple messages concatenated with $$__$$

    eg. osi_protobuf_encoded_string_1$$__$$osi_protobuf_encoded_string_2$$__$$osi_protobuf_encoded_string_3
    """
    def check_length(self):
        """
        Check if number of messages found in a data file is the same in data_file_B as in data_file_A

        :return: True if number is match False otherwise
        """
        # Check of number of elements in both data structures
        length_of_data_a = len(self.data_a.keys())
        length_of_data_b = len(self.data_b.keys())

        if length_of_data_a != length_of_data_b:
            logging.debug(colored('Length mismatch! Aborting comparision!', 'red'))
            sys.exit(-1)
            return False
        else:
            logging.info(colored('Length is matched!', 'green'))
            return True
        return True

    def check_timestamps(self):
        """
        For every message form data_a find a timestamp and check if it is present in data_b
        Log the findings to the logger instance.

        :return: None
        """
        # Checking if data has the same timestamps
        list_of_dataA_timestamps = self.data_a.keys()
        list_of_dataB_timestamps = self.data_b.keys()
        sort(list_of_dataA_timestamps)

        for element in list_of_dataA_timestamps:
            if element not in list_of_dataB_timestamps:
                logging.debug(colored('Timestamp match for {} and {} at {}s  [Not OK]' \
                                      .format(self.get_filename_a(), self.get_filename_b(), element), 'red'))
            else:
                logging.info(colored('Timestamp match for {} and {} at {}s  [OK]' \
                                     .format(self.get_filename_a(), self.get_filename_b(), element), 'green'))

    def check_message_content(self):
        """
        Two messages with matching timestamp are compared. To perform this task
        1. Instance of CompareMessages is created
        2. Method compare_vehicle list is invoked
        3. Method compare_stationary_object is invoked

        If one needs to extend the scope of message comparision additional methods should be added to
        CompareMessages and invoked here.

        :return: None
        """
        key_list = self.data_a.keys()
        sort(key_list)

        for key in key_list:
            # We already assume that key exist in both data_a and data_b
            logging.debug(colored('Checking message content for timestamp {}s.'.format(key), 'white'))
            message_comparision = CompareMessages(self.data_a[key], self.data_b[key], key)
            message_comparision.compare_vehicle_list()
            message_comparision.compare_stationary_object()

    def read_data(self):
        """
        This method loads the data from file and put it in the Class's data structure

        This method uses superclass's (Comparision) data fields.
        self.data_file_a
        self.data_file_a
        self.read_message_limit

        1. split_file() function load encoded data from file. Split data into encoded messages.Put messages into a list.
        2. get_timestamped_data() parse osi_protobuf string into a data structure. Create key based on timestamp.
            Put the data into a dictionary.
        3. this dictionary is stored in Class's data structure.

        :return: None
        """
        messgaes_a = split_file(self.data_file_a)
        messgaes_b = split_file(self.data_file_b)

        # Imposing limit on number of messages read
        if self.read_message_limit:
            if self.read_message_limit < len(messgaes_a):
                messgaes_a = messgaes_a[:self.read_message_limit]
                messgaes_b = messgaes_b[:self.read_message_limit]

        logging.info("Reading file {}".format(self.data_file_a))
        self.data_a = get_timestamped_data(messgaes_a)

        logging.info("Reading file {}".format(self.data_file_b))
        self.data_b = get_timestamped_data(messgaes_b)


class CompareMessages(Comparision):
    """
    The class holds osi data and enable the comparision of two osi messages.

    Currently two methods are implemented.
    compare_vehicle_list()
    compare_stationary_object()

    This class uses superclass's (Comparision) data fields.
    self.data_file_a
    self.data_file_a
    self.read_message_limit

    """
    def __init__(self, message_a, message_b, timestamp):
        self.a = message_a
        self.b = message_b
        self.missing_vehicle_list = []
        self.missing_object_list = []
        self.timestamp = timestamp

    def compare_vehicle_list(self):
        """
        Check if vehicles from message a match the vehicles from dataset b.

        The values stored in other fields are ignored.

        If the vehicle from message a is not found in b or is mismatched it is added to Class's attrubute
        self.missing_vehicle_list()

        :return: None
        """
        vehicles_a_dictionary = create_id_dictionary(self.a.ground_truth.global_ground_truth.vehicle)
        vehicles_b_dictionary = create_id_dictionary(self.b.ground_truth.global_ground_truth.vehicle)

        for vehicle_id in vehicles_a_dictionary.keys():

            # Data set B should have all vehicles form data set A
            if vehicle_id not in vehicles_b_dictionary.keys():
                tmp_string_ = "Vehicle ID:{} EXIST for {} and {}      [NOT OK!]" \
                                      .format(vehicle_id, self.get_filename_a(), self.get_filename_b())
                logging.debug(colored(tmp_string_, 'red'))
                self.missing_vehicle_list.append(vehicle_id)

            else:
                logging.debug(colored('Vehicle ID:{} EXIST for {} and {}      [OK]' \
                                     .format(vehicle_id, self.get_filename_a(), self.get_filename_b()), 'green'))
                vehicle_a = vehicles_a_dictionary[vehicle_id]
                vehicle_b = vehicles_b_dictionary[vehicle_id]
                vehicle_comparision = CompareVehicles(vehicle_a, vehicle_b)
                is_vehicle_same = True
                is_vehicle_same = is_vehicle_same and vehicle_comparision.compare_type()
                is_vehicle_same = is_vehicle_same and vehicle_comparision.compare_base()
                if not is_vehicle_same:
                    tmp_string_ = 'Vehicle ID {} at Timestamp {} is different in data sets.'. \
                                          format(vehicle_id, self.timestamp)
                    logging.debug(colored(tmp_string_, 'red'))
                    self.missing_vehicle_list.append(tmp_string_)

        number_of_failed_objects = len(self.missing_object_list)
        if number_of_failed_objects == 0:
            logging.info(colored('All vehicles for timestamp {} matched'.format(self.timestamp), 'green'))

    def compare_stationary_object(self):
        stationary_objects_a_dictionary = \
            create_id_dictionary(self.a.ground_truth.global_ground_truth.stationary_object)
        stationary_objects_b_dictionary = \
            create_id_dictionary(self.b.ground_truth.global_ground_truth.stationary_object)

        for object_id in stationary_objects_a_dictionary.keys():

            # Data set B should have all objects form data set A
            if object_id not in stationary_objects_b_dictionary.keys():
                temp_string_ = "Object ID:{} EXIST for {} and {}      [NOT OK!]" \
                                      .format(object_id, self.get_filename_a(), self.get_filename_b())
                temp_string_ = colored(temp_string_, 'red')
                logging.debug(colored(temp_string_, 'red'))
                self.missing_object_list.append(temp_string_)
            else:
                logging.debug(colored('Object ID:{} EXIST for {} and {}      [OK]' \
                                     .format(object_id, self.get_filename_a(), self.get_filename_b()), 'green'))
                object_a = stationary_objects_a_dictionary[object_id]
                object_b = stationary_objects_b_dictionary[object_id]
                stationary_object_comparision = CompareStationaryObjects(object_a, object_b)
                is_object_same = True
                is_object_same = is_object_same and stationary_object_comparision.compare_type()
                is_object_same = is_object_same and stationary_object_comparision.compare_base()
                if not is_object_same:
                    temp_string_ = 'Object ID {} at Timestamp {} is different in both data set.'.\
                                format(object_id, self.timestamp)
                    logging.debug(colored(temp_string_, 'red'))
                    self.missing_object_list.append(temp_string_)

        number_of_failed_objects = len(self.missing_object_list)
        if number_of_failed_objects == 0:
            logging.info(colored('All objects  for timestamp {} matched'.format(self.timestamp), 'green'))


class CompareVehicles(Comparision):
    """
    The purpose of the class is to facilitate comparison of two osi.vehicle data structures.

    """
    def __init__(self, vehicle_a, vehicle_b):
        self.vehicle_a = vehicle_a
        self.vehicle_b = vehicle_b

    def compare_type(self):
        if self.vehicle_a.type != self.vehicle_b.type:
            logging.debug(colored('Vehicle ID:{} TYPE match for {} and {} [NOT OK!] ' \
                                  .format(self.vehicle_a.id.value, self.get_filename_a(), self.get_filename_b()),
                                  'red'))
            return False
        else:
            logging.debug(colored('Vehicle ID:{} TYPE match for {} and {} [OK] ' \
                                 .format(self.vehicle_a.id.value, self.get_filename_a(), self.get_filename_b()),
                                 "green"))
            return True

    def compare_base(self):

        base_comparision = CompareBase(self.vehicle_a.base, self.vehicle_b.base)
        result = True
        result = result and base_comparision.compare_dimension()
        result = result and base_comparision.compare_position()
        result = result and base_comparision.compare_velocity()
        result = result and base_comparision.compare_acceleration()
        result = result and base_comparision.compare_orientation()
        result = result and base_comparision.compare_orientation_rate()
        return result


class CompareStationaryObjects(Comparision):
    """
    The purpose of the class is to facilitate comparison of two osi.stationary_object data structures.

    """
    def __init__(self, object_a, object_b):
        self.object_a = object_a
        self.object_b = object_b

    def compare_type(self):
        """
        :return: True if both stationary objects have matching ID
        """
        if self.object_a.type != self.object_b.type:
            logging.debug(colored('Object ID:{} TYPE match for {} and {} [NOT OK!] ' \
                                  .format(self.object_a.id.value, self.get_filename_a(), self.get_filename_b()), 'red'))
            return False
        else:
            logging.debug(colored('Object ID:{} TYPE match for {} and {} [OK] ' \
                                .format(self.object_a.id.value, self.get_filename_a(), self.get_filename_b()), "green"))
            return True

    def compare_base(self):
        """
        :return: Return true if the object has matching base's components
        """
        base_comparision = CompareBase(self.object_a.base, self.object_b.base)
        result = True
        result = result and base_comparision.compare_dimension()
        result = result and base_comparision.compare_position()
        result = result and base_comparision.compare_orientation()
        return result


class CompareBase(Comparision):
    """
    The purpose of this class is to compare osi_base, and log the result to logger.
    """
    def __init__(self, base_a, base_b):
        self.base_a = base_a
        self.base_b = base_b

    def compare_velocity(self):
        """
        The method uses float comparision method form superclass.

        :return: True if velocity is matched in all three components, False otherwise
        """
        result = True
        if not self.are_floats_equal(self.base_a.velocity.x, self.base_b.velocity.x):
            logging.debug('Base.velocity.x mismatch {} {}'.format(self.base_a.velocity.x, self.base_b.velocity.x))
            result = False
        if not self.are_floats_equal(self.base_a.velocity.y, self.base_b.velocity.y):
            logging.debug('Base.velocity.y mismatch {} {}'.format(self.base_a.velocity.y, self.base_b.velocity.y))
            result = False
        if not self.are_floats_equal(self.base_a.velocity.z, self.base_b.velocity.z):
            logging.debug('Base.velocity.x mismatch {} {}'.format(self.base_a.velocity.z, self.base_b.velocity.z))
            result = False
        return result

    def compare_position(self):
        """
        The method uses float comparision method form superclass.

        :return: True if position is matched in all three components, False otherwise
        """
        is_matching = True
        if not self.are_floats_equal(self.base_a.position.x, self.base_b.position.x):
            logging.debug('Base.position.x mismatch {} {}'.format(self.base_a.position.x, self.base_b.position.x))
            is_matching = False
        if not self.are_floats_equal(self.base_a.position.y, self.base_b.position.y):
            logging.debug('Base.position.y mismatch {} {}'.format(self.base_a.position.y, self.base_b.position.y))
            is_matching = False
        if not self.are_floats_equal(self.base_a.position.z, self.base_b.position.z):
            logging.debug('Base.position.z mismatch {} {}'.format(self.base_a.position.z, self.base_b.position.z))
            is_matching = False
        return is_matching

    def compare_orientation(self):
        """
        The method uses float comparision method form superclass.

        :return: True if orientation is matched in all three components, False otherwise
        """
        result = True
        if not self.are_floats_equal(self.base_a.orientation.roll, self.base_b.orientation.roll):
            logging.debug('Base.orientation.roll mismatch {} {}'.format(self.base_a.orientation.roll,
                                                                        self.base_b.orientation.roll))
            result = False
        if not self.are_floats_equal(self.base_a.orientation.pitch, self.base_b.orientation.pitch):
            logging.debug('Base.orientation.pitch mismatch {} {}'.format(self.base_a.orientation.pitch,
                                                                         self.base_b.orientation.pitch))
            result = False
        if not self.are_floats_equal(self.base_a.orientation.yaw, self.base_b.orientation.yaw):
            logging.debug(
                'Base.orientation.yaw mismatch {} {}'.format(self.base_a.orientation.yaw, self.base_b.orientation.yaw))
            result = False
        return result

    def compare_dimension(self):
        """
        The method uses float comparision method form superclass.

        :return: True if dimension is matched in all three components, False otherwise
        """
        result = True
        if not self.are_floats_equal(self.base_a.dimension.width, self.base_b.dimension.width):
            logging.debug('Base.dimension.width mismatch {} {}'. \
                          format(self.base_a.dimension.width, self.base_b.dimension.width))
            result = False
        if not self.are_floats_equal(self.base_a.dimension.length, self.base_b.dimension.length):
            logging.debug('Base.dimension.length mismatch {} {}'. \
                          format(self.base_a.dimension.length, self.base_b.dimension.length))
            result = False
        if not self.are_floats_equal(self.base_a.dimension.height, self.base_b.dimension.height):
            logging.debug('Base.dimension.height mismatch {} {}'. \
                          format(self.base_a.dimension.height, self.base_b.dimension.height))
            result = False
        return result

    def compare_acceleration(self):
        """
        The method uses float comparision method form superclass.

        :return: True if acceleration is matched in all three components, False otherwise
        """
        result = True
        if not self.are_floats_equal(self.base_a.acceleration.x, self.base_b.acceleration.x):
            logging.debug('Base.acceleration.x mismatch {} {}' \
                          .format(self.base_a.acceleration.x, self.base_b.acceleration.x))
            result = False
        if not self.are_floats_equal(self.base_a.acceleration.y, self.base_b.acceleration.y):
            logging.debug('Base.acceleration.x mismatch {} {}' \
                          .format(self.base_a.acceleration.x, self.base_b.acceleration.x))
            result = False
        if not self.are_floats_equal(self.base_a.acceleration.z, self.base_b.acceleration.z):
            logging.debug('Base.acceleration.x mismatch {} {}' \
                          .format(self.base_a.acceleration.x, self.base_b.acceleration.x))
            result = False
        return result

    def compare_orientation_rate(self):
        """
        The method uses float comparision method form superclass.

        :return: True if orientation_rate is matched in all three components, False otherwise
        """
        result = True
        if not self.are_floats_equal(self.base_a.orientation_rate.roll, self.base_b.orientation_rate.roll):
            logging.debug('Base.orientation_rate.roll mismatch {} {}'. \
                          format(self.base_a.orientation_rate.roll, self.base_b.orientation_rate.roll))
            result = False
        if not self.are_floats_equal(self.base_a.orientation_rate.pitch, self.base_b.orientation_rate.pitch):
            logging.debug('Base.orientation_rate.pitch mismatch {} {}'. \
                          format(self.base_a.orientation_rate.pitch, self.base_b.orientation_rate.pitch))
            result = False
        if not self.are_floats_equal(self.base_a.orientation_rate.yaw, self.base_b.orientation_rate.yaw):
            logging.debug('Base.orientation_rate.yaw mismatch {} {}'. \
                          format(self.base_a.orientation_rate.yaw, self.base_b.orientation_rate_rate.yaw))
            result = False
        return result


def get_timestamped_data(messages):
    """
    This function receives a list with osi-encoded-strings
    Parses these stings into osi data structure
    And puts those into dictionary.
    The key of the dictionary is a timestamp in nanoseconds

    :param messages: a list of osi strings:
    :return dictionary with key:simulation time in nanoseconds :
    """
    import osi.osi_sensordata_pb2
    dictionary = {}

    total_number_of_messages = len(messages)
    counter = 0

    for message in messages:
        progress = 100 * counter / total_number_of_messages
        counter += 1
        sensor_data = osi.osi_sensordata_pb2.SensorData()
        sensor_data.ParseFromString(message)

        seconds = sensor_data.ground_truth.global_ground_truth.timestamp.seconds
        nanos = sensor_data.ground_truth.global_ground_truth.timestamp.nanos

        key_string = "{:04d}.{:09d}".format(seconds, nanos)
        logging.info("Reading file {}% Done!".format(progress))
        dictionary[key_string] = sensor_data
    return dictionary


def create_id_dictionary(data):
    """
    This function creates the directory and puts as keys ids

    :param data: iterable data that have data.id.value field
    :return:
    """
    dictionary = {}
    for obj in data:
        dictionary[obj.id.value] = obj
    return dictionary


def split_file(input_file):
    """
    Separate osi messages. Into a list of strings.
    
    This function is not python3 safe.

    Possible improvement. If multiple '$$__$$' separators occur in one line it will not work correctly!
    """

    # Open the file and read content""""""
    f = open(input_file, 'rb')
    lines = f.readlines()
    f.close()

    # Search for a
    backup = ''
    raw_data = ''
    list_of_string_messages = []
    for line in lines:
        index = line.find('$$__$$')

        if backup != '':
            raw_data = backup
            backup = ''

        if index == -1:
            raw_data += line

        if index > 0:
            raw_data += line[:index]
            backup = line[index + 6:]
            list_of_string_messages.append(raw_data)
            raw_data = ''

    # push the last message to the list
    if raw_data != '':
        list_of_string_messages.append(raw_data)

    return list_of_string_messages


def main():

    # Create parser, add arguments, parse
    parser = argparse.ArgumentParser(description='Compare two set of OSI data.')
    parser.add_argument('file_a', type=str, help='File to be a benchmark')
    parser.add_argument('file_b', type=str, help='File to be a be investigated for missing data')
    parser.add_argument('-rl', '--read_message_limit', type=int, help='Set limit to the number of messages read')
    parser.add_argument("-v", "--verbose", help="increase output verbosity", action="store_true")
    arguments = parser.parse_args()
    if arguments.verbose:
        logging.basicConfig(format='%(message)s', level=logging.DEBUG)
    else:
        logging.basicConfig(format='%(message)s', level=logging.INFO)

    # Assign command line arguments
    Comparision.data_file_a = arguments.file_a
    Comparision.data_file_b = arguments.file_b
    Comparision.read_message_limit = arguments.read_message_limit

    # perform comparision
    compare_data = CompareData()
    logging.info('Reading data started.')
    compare_data.read_data()
    logging.info('Reading data finished.')

    logging.info('Checking length of files.')
    compare_data.check_length()
    logging.info('Checking length had been done.')

    logging.info('Checking timestamps consistency.')
    compare_data.check_timestamps()
    logging.info('Timestamps checking finished.')

    logging.info('Checking messages content.')
    compare_data.check_message_content()
    logging.info('Message checking finished.')


if __name__ == '__main__':
    main()
