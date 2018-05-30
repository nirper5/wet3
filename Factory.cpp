//
// Created by Arkadi Gurevich on 26/05/2018.
//

#include "Factory.h"

typedef struct produce_t{
    int num_products;
    Product* products;
} Produce;


typedef struct company_buyer_t{
    std::list<Product> products;
    unsigned int id;
    int num_products;
    Factory* factory;
} CompanyBuyer;


typedef struct thief_t{
    int num_products;
    unsigned int fake_id;
} Thief;

//static void* wrapper_returnProducts(void* crp){
//    Company_return_products* struct_crp  = static_cast<Company_return_products*->crp;
//    std::list<std::string> products(struct_crp->products);
//    unsigned int id = 0;
//    return products(products,id);
//
//}



//static void* wrapper_produce(void* new_produce){
//    Produce* new_produce_c = static_cast<Produce*> (new_produce);
//    int num_products = new_produce_c->num_products;
//    Product* products = new_produce_c->products;
//    return produce(num_products, products);
//}

//static void* wrapper_stealProducts(void* sp){
//
//    Thief* struct_sp  = static_cast<Thief*>(sp);
//    int num_products = struct_sp->num_products;
//
//    unsigned int fake_id = struct_sp->fake_id;
//    int stolen_products = factory.stealProducts(num_products,fake_id);
//    return (void*)(stolen_products);
//
//}

void* wrapper_buyProducts(void* company){



    CompanyBuyer* company_buyer = static_cast<CompanyBuyer*>(company);
    int number_of_products = company_buyer->num_products;
    Factory* factory = company_buyer->factory;
    std::list<Product>* purchased_products = new std::list<Product>;


    (*purchased_products) = factory->buyProducts(number_of_products);



    // TESTING ARKADI
    std::list<Product>::iterator it_;
    for ( it_ = (*purchased_products).begin(); it_ != (*purchased_products).end(); it_++ )
        std::cout << it_->getId() << std::endl ;

    return purchased_products;

}




Factory::Factory(){

    factory_open = true;
    return_service_open = true;
    num_of_thieves = 0;
    num_of_waiting_thieves = 0;
    num_of_companies= 0;
    num_of_buyers= 0;
    pthread_cond_init(&thieves, NULL);
    pthread_cond_init(&companies, NULL);
    pthread_mutex_init(&global_lock,NULL);


}

Factory::~Factory(){

    for (std::map<unsigned int, pthread_t>::iterator it = Threads.begin(); it != Threads.end(); ++it)
        pthread_cancel(it->second);

}

void Factory::startProduction(int num_products, Product* products,unsigned int id){
}

void Factory::produce(int num_products, Product* products){
}

void Factory::finishProduction(unsigned int id){
}

void Factory::startSimpleBuyer(unsigned int id){
}

int Factory::tryBuyOne(){
    return -1;
}

int Factory::finishSimpleBuyer(unsigned int id){
    return -1;
}

void Factory::startCompanyBuyer(int num_products, int min_value,unsigned int id){


    // (ARKADI ) for TESTING  Insert num_products elements to the end of the list
    for (int i = 0; i < num_products ; ++i) {
        Product product(i,10);
        Available.push_back(product);
    }

    std::list<Product>::iterator it_;
    for ( it_ = (Available).begin(); it_ != (Available).end(); it_++ )
        std::cout << it_->getId() << std::endl ;

    // Check arguments
    if( num_products < 0 || min_value < 0 )
        return;

    // We will create a structure that will hold the fields we want to send to the function below
    CompanyBuyer* company_buyer = new CompanyBuyer;
    company_buyer->factory = this;
    company_buyer->num_products=num_products;

    // Here I create the thread and call the wrapper function and get the answer in the purchased_products_ptr
    pthread_t pthread_buyProducts;
    pthread_create(&pthread_buyProducts, NULL, &wrapper_buyProducts,company_buyer); // create new thread
    Threads.insert(std::pair<unsigned int ,pthread_t>(id,pthread_buyProducts)); // insert thread with id to map
    std::list<Product>* purchased_products_ptr; // A pointer will receive the result of the thread
    pthread_join(pthread_buyProducts, (void**)&purchased_products_ptr);
    company_buyer->products.assign((*purchased_products_ptr).begin(), (*purchased_products_ptr).end());// Copy list to local std::list
    delete(purchased_products_ptr); // free memory



// ( ARKADI ) TESTING - traversing the list from the end to begin
//    for (std::list<Product>::reverse_iterator it=company_buyer->products.rbegin(); it!=company_buyer->products.rend(); ++it) // traverse list from end to begin
//        std::cout << it->getId() << std::endl ;


}

std::list<Product> Factory::buyProducts(int num_products){

    // Waiting until it was his turn to buy the products
    pthread_mutex_lock(&global_lock);
    while ( num_of_thieves > 0  || num_of_waiting_thieves > 0 || Available.size() < num_products  || factory_open == false || num_of_companies > 0 )
        pthread_cond_wait(&companies, &global_lock);
    num_of_companies++;
    std::list<Product> purchased;
    purchased.assign(std::next(Available.begin(), 0), std::next(Available.begin() , 2));

    return std::list<Product>(purchased);
}

void Factory::returnProducts(std::list<Product> products,unsigned int id){
}

int Factory::finishCompanyBuyer(unsigned int id){
    return 0;
}

void Factory::startThief(int num_products,unsigned int fake_id){
}

int Factory::stealProducts(int num_products,unsigned int fake_id){
    return 0;
}



int Factory::finishThief(unsigned int fake_id){
    return 0;
}

void Factory::closeFactory(){
}

void Factory::openFactory(){
}

void Factory::closeReturningService(){
}

void Factory::openReturningService(){
}

std::list<std::pair<Product, int>> Factory::listStolenProducts(){
    return std::list<std::pair<Product, int>>();
}

std::list<Product> Factory::listAvailableProducts(){
    return std::list<Product>(Available);
}






