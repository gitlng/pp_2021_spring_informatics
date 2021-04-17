// Copyright 2021 Tkachev Alexey
#include <random>
#include <ctime>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <climits>
#include <utility>
#include <omp.h>
#include <iostream>

#include "../../../modules/task_2/tkachev_a_graham_pass/graham_pass.h"

std::vector<Point> getRandomizedVector(double from,
                        double up_to, uint32_t count) {
    assert(from < up_to);
    assert(count > 0);
    std::mt19937 random_generator(time(NULL));
    std::uniform_real_distribution<double> real_dist(from, up_to);
    std::vector<Point> randomized_vector(count);

    for (uint32_t i = 0; i < count; i++) {
        randomized_vector[i].x = real_dist(random_generator);
        randomized_vector[i].y = real_dist(random_generator);
    }

    return randomized_vector;
}

void swapPoints(Point* p1, Point* p2) {
    std::swap(p1->x, p2->x);
    std::swap(p1->y, p2->y);
}

Point nextToTopOfStack(std::stack<Point> points) {
    assert(!points.empty());
    const Point tmp = points.top();
    points.pop();
    const Point result = points.top();
    points.push(tmp);
    return result;
}

double angleThreePoints(Point p1, Point p2, Point p3) {
     return (p2.x - p1.x) * (p3.y - p1.y) -
            (p2.y - p1.y) * (p3.x - p1.x);
}

double distanceTwoPoints(Point p1, Point p2) {
    return sqrt((p2.x - p1.x) * (p2.x - p1.x) +
                (p2.y - p1.y) * (p2.y - p1.y));
}

std::vector<Point> sortedByPolarAngle(std::vector<Point> points, bool multithreading) {
   
    assert(!points.empty());

    if (multithreading) {
            #pragma omp parallel
            {
                int chunk;
                if (points.size() < 100) {
                    chunk = ceil(points.size() / omp_get_num_threads());
                } else {
                    chunk = (points.size() / omp_get_num_threads()) / 100;
                }
                
                for (uint64_t repeat = 2; repeat < points.size() + omp_get_num_threads(); repeat++) {
                    #pragma omp for schedule(dynamic, chunk)
                    for (uint64_t j = 2; j < points.size(); j++) {

                        double current_angle = angleThreePoints(
                                        points[j-1], points[j], points[0]);
            
                        if (current_angle <= 0) {
                            swapPoints(&points[j-1], &points[j]);
                        } else {
                            if (current_angle == 0) {
                                if (distanceTwoPoints(points[j], points[0]) <
                                    distanceTwoPoints(points[j-1], points[0])) {

                                    swapPoints(&points[j-1], &points[j]);                        
                                }
                            }
                        }
                    }
                }
            }
    } else {
        for (uint64_t i = 2; i < points.size(); i++) {
            for (uint64_t j = 2; j < points.size(); j++) {

                double current_angle = angleThreePoints(
                                    points[j-1], points[j], points[0]);

                if (current_angle <= 0) {
                    swapPoints(&points[j-1], &points[j]);
                } else {
                    if (current_angle == 0) {
                        if (distanceTwoPoints(points[j], points[0]) <
                            distanceTwoPoints(points[j-1], points[0])) {
                            swapPoints(&points[j-1], &points[j]);
                        }
                    }
                }
            }
        }
    }
    return points;
}

uint32_t getIndexMinLeftDownPoint(std::vector<Point> points, bool multithreading) {

    assert(!points.empty());

    double min_x = std::numeric_limits<double>::max();
    double min_y = std::numeric_limits<double>::max();

    std::vector<double> min_xs, min_ys;

    if (multithreading) {

        #pragma omp parallel firstprivate(points)
        {
            std::vector<double> min_ys(omp_get_num_threads(), 
                        std::numeric_limits<double>::max());

            #pragma omp for schedule(static, 2)
            for (uint32_t i = 0; i < points.size(); i++) {
                if (min_ys[omp_get_thread_num()] > points[i].y) {
                    min_ys[omp_get_thread_num()] = points[i].y;
                } 
            }

            #pragma omp master
            {
                min_y = *std::min_element(min_ys.begin(), min_ys.end());
            }
        }
            
        #pragma omp parallel firstprivate(points)
        {
            std::vector<double> min_xs(omp_get_num_threads(),
                            std::numeric_limits<double>::max());

            min_xs[omp_get_thread_num()] = min_x;

            #pragma omp for schedule(static, 2)
            for (uint32_t i = 0; i < points.size(); i++) {
                if (points[i].y == min_y && 
                    min_xs[omp_get_thread_num()] > points[i].x) {

                    // std::cout << "points[y] = "<<points[i].y <<" min_xs = " << min_xs[omp_get_thread_num()] << " > points[i].x = " << points[i].x << "true? : " << (points[i].y == min_y) << " | "  <<(min_xs[omp_get_thread_num()] > points[i].x) << std::endl;

                    min_xs[omp_get_thread_num()] = points[i].x;
                } 
                
            }

            #pragma omp master
            {
                // for(auto p : min_xs) std::cout << " XS: " << p;
                min_x = *std::min_element(min_xs.begin(), min_xs.end());
            }

        }
    } else {
        for (uint32_t i = 0; i < points.size(); i++) {
            if (points[i].y < min_y) {
                min_y = points[i].y;
            }
        }
        for (uint32_t i = 0; i < points.size(); i++) {
            if (points[i].y == min_y) {
                if (points[i].x < min_x) {
                    min_x = points[i].x;
                }
            }
        }
    }

    // if (multithreading ) std::cout << "minx " << min_x << "min y " <<min_y << std::endl;


    for (uint64_t i = 0; i < points.size(); i++) {
        if (points[i].x == min_x && points[i].y == min_y) {
            // std::cout << "HELLLLLLLLLO" << std::endl;
            return i;
        }
    }
    throw;
}

std::stack<Point> useGrahamAlgorithm(std::vector<Point> points, bool multithreading) {

    assert(!points.empty()); 

    std::stack<Point> points_stack;

    if (points.size() <= 3) {
        for (uint32_t i = 0; i < points.size(); i++) {
            points_stack.push(points[i]);
        }
        points_stack.push(points[0]);
        return points_stack;
    }

    swapPoints(&points[0],
        &points[getIndexMinLeftDownPoint(points, multithreading)]);

    points = sortedByPolarAngle(points, multithreading);

    // if (multithreading) points = sortedByPolarAngle(points, false);

    // for (auto item : points) std::cout << item.x << "." << item.y << " ";
    // std::cout<<std::endl;

    points.push_back(points[0]);

    points_stack.push(points[0]);
    points_stack.push(points[1]);
    points_stack.push(points[2]);

    for (uint32_t i = 3; i < points.size(); i++) {
        while (
            angleThreePoints(
                nextToTopOfStack(points_stack),
                points_stack.top(),
                points[i]) <= 0
        ){
            if (!points_stack.empty()) {
                points_stack.pop();
            }   
        }

        points_stack.push(points[i]);
    }

    return points_stack;
}