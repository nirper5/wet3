
//
// Created by Arkadi Gurevich on 26/05/2018.
//

#ifndef FACTORY_H_
#define FACTORY_H_

#include <pthread.h>
#include <list>
#include "Product.h"
#include <bits/stdc++.h>

class Factory{
public:
    Factory();
    ~Factory();

    void startProduction(int num_products, Product* products, unsigned int id);
    void produce(int num_products, Product* products);
    void finishProduction(unsigned int id);

    void startSimpleBuyer(unsigned int id);
    int tryBuyOne();
    int finishSimpleBuyer(unsigned int id);

    void startCompanyBuyer(int num_products, int min_value,unsigned int id);
    std::list<Product> buyProducts(int num_products);
    void returnProducts(std::list<Product> products,unsigned int id);
    int finishCompanyBuyer(unsigned int id);

    void startThief(int num_products,unsigned int fake_id);
    int stealProducts(int num_products,unsigned int fake_id);
    int finishThief(unsigned int fake_id);

    void closeFactory();
    void openFactory();

    void closeReturningService();
    void openReturningService();

    std::list<std::pair<Product, int>> listStolenProducts();
    std::list<Product> listAvailableProducts();





private:


    bool factory_open;
    bool return_service_open;

    std::map<unsigned int  , pthread_t> Threads;
    std::list<Product> Available;
    std::list<std::pair<Product, int>> Stolen;

    int num_of_thieves;
    int num_of_waiting_thieves;
    int num_of_companies;
    int num_of_buyers;

    pthread_cond_t thieves;
    pthread_cond_t companies;
    pthread_mutex_t global_lock;






};


#endif // FACTORY_H_
