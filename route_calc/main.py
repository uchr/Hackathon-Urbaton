import json
import xml.etree.ElementTree as ET

import os
import time
import requests


def get_route_tile(min_lat, min_lon, max_lat, max_lon, out_file):
    url = f"https://api.openstreetmap.org/api/0.6/map&bbox={min_lon},{min_lat},{max_lon},{max_lat}"
    response = requests.get(url)
    print(response.url)
    with open(out_file, "w") as file:
        file.write(response.text)


def download_tile():
    """
    https://api.openstreetmap.org/api/0.6/map?bbox=82.54715,54.839455,83.182984,55.103517
    """
    city_min_lat = 54.870285
    city_min_lon = 82.553329
    city_max_lat = 55.230589
    city_max_lon = 83.299713

    step = 0.01
    for w_index in range(int((city_max_lat - city_min_lat) / step) + 1):
        for h_index in range(int((city_max_lon - city_min_lon) / step) + 1):

            min_lat = city_min_lat + w_index * step
            min_lon = city_min_lon + h_index * step
            max_lat = city_min_lat + (w_index + 1) * step
            max_lon = city_min_lat + (h_index + 1) * step

            file_name = os.path.join("./map", "_".join(map(str, [w_index, h_index])))
            get_route_tile(min_lat, min_lon, max_lat, max_lon, file_name)

            time.sleep(0.1)


class Point:

    lat = ""
    lon = ""
    # weight = 0

    def __init__(self, lat, lon, weight=0):
        self.lat = lat
        self.lon = lon
        # self.weight = weight

    # def __json__(self):
    #     return json.dump({"lat": self.lat, "lon": self.lon, "weight": self.weight})

class Way:
    points = []
    type = ""


class JSONEncoder(json.JSONEncoder):
    def default(self, obj):
        if hasattr(obj, '__json__'):
            return obj.__json__()
        elif hasattr(obj, '__getitem__') and hasattr(obj, 'keys'):
            return dict(obj)
        elif hasattr(obj, '__dict__'):
            return {member: getattr(obj, member)
                    for member in dir(obj)
                    if not member.startswith('_') and
                    not hasattr(getattr(obj, member), '__call__')}

        return json.JSONEncoder.default(self, obj)

def main():

    nodes = {}

    tree = ET.parse('./highways.osm')
    root = tree.getroot()

    for node in tree.iter("node"):
        nodes[node.get("id")] = (node.get("lat"), node.get("lon"))

    ways = {}

    for way in root.iter("way"):
        route_way = Way()
        route_way.type = way.get("highway")
        for nd in way.iter("nd"):
            node = nodes[nd.get("ref")]
            route_way.points.append(Point(node[0], node[1]))
        ways[way.get("id")] = route_way

    with open("graph_dump.json", "w") as file:
        json.dump(ways, file, cls=JSONEncoder)

def dump_road():

    nodes = {}

    tree = ET.parse('./highways.osm')
    root = tree.getroot()

    for node in tree.iter("node"):
        nodes[node.get("id")] = (node.get("lat"), node.get("lon"))

    ways = []

    for way in root.iter("way"):
        route = []
        for nd in way.iter("nd"):
            node = nodes[nd.get("ref")]
            route.append((node[0], node[1]))

        route_way = {}

        for tag in way.iter("tag"):
            if tag.get("k") == "highway":
                route_way["type"] = tag.get("v")
                break

        route_way["points"] = route
        ways.append(route_way)

    with open("graph_dump.json", "w") as file:
        json.dump(ways, file)



def dump_points():

    new_point_max_id = 1000000000

    type_weight = {
        "motorway" : 2,
        "trunk" : 2,
        "primary" : 5,
        "secondary" : 4,
        "tertiary" : 3,
        # "unclassified" : 1,
        # "residential" : 1,
    }

    nodes = {}

    tree = ET.parse('./highways.osm')
    root = tree.getroot()

    for node in tree.iter("node"):
        nodes[node.get("id")] = (node.get("lat"), node.get("lon"))

    points = {}

    for way in root.iter("way"):

        for tag in way.iter("tag"):
            if tag.get("k") == "highway":
                if tag.get("v") not in type_weight:
                    current_type_weight = None
                    break
                current_type_weight = type_weight[tag.get("v")]
                break

        if current_type_weight == None:
            continue

        if current_type_weight >= 4:
            node_ids = list(way.iter("nd"))
            for nd, next_nd in zip(node_ids, node_ids[1:]):
                node = nodes[nd.get("ref")]
                next_node = nodes[next_nd.get("ref")]

                weight = current_type_weight
                if nd.get("ref") in points:
                    weight = max(points[nd.get("ref")].weight, current_type_weight)
                if next_nd.get("ref") in points:
                    weight = max(points[next_nd.get("ref")].weight, current_type_weight)

                for point in generate_points(node, next_node, weight, 10):
                    points[new_point_max_id] = point
                    new_point_max_id = new_point_max_id - 1


        for nd in way.iter("nd"):
            node = nodes[nd.get("ref")]
            if nd.get("ref") in points:
                if points[nd.get("ref")].weight < current_type_weight:
                    points[nd.get("ref")].weight = current_type_weight
            else:
                points[nd.get("ref")] = Point(node[0], node[1], current_type_weight)

    # print(len(points.values()))
    #
    # with open("graph_dump.json", "w") as file:
    #     json.dump(list(points.values()), file, cls=JSONEncoder)

    with open("graph_dump.csv", "w") as file:
        file.write("lat;lon;weight\n")
        for point in points.values():
            file.write(f"{point.lat};{point.lon};{point.weight}\n")

def generate_points(start, finish, weight, count=3):
    lat_finish = float(finish[0])
    lat_start = float(start[0])

    lon_finish = float(finish[1])
    lon_start = float(start[1])

    lat_len = abs(lat_finish - lat_start)
    lon_len = abs(lon_finish - lon_start)

    lat_step = lat_len / (count + 1)
    lon_step = lon_len / (count + 1)

    points = []

    for index in range(1, count + 1):
        points.append(Point(lat_start + lat_step * index, lon_start + lon_step * index, weight))

    return points

if __name__ == '__main__':
    dump_road()
