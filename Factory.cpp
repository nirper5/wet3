//
// Created by Arkadi Gurevich on 26/05/2018.
//

#include <unistd.h>
#include "Factory.h"

typedef struct produce_t{
    int num_products;
    Product* products;
} Produce;


typedef struct company_buyer_t{
    int num_products;
    int min_value;
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

static void* wrapper_CompanyBuyer(void* company){


    // casting to company_buyer
    CompanyBuyer* company_buyer = static_cast<CompanyBuyer*>(company);

    // Extracts the fields
    int number_of_products = company_buyer->num_products;
    Factory* factory = company_buyer->factory;
    int min_value = company_buyer->min_value;

    // free memory of struct because we will not use it again
    delete company_buyer;
    company_buyer = nullptr;

    // A list containing the purchased products
    std::list<Product> purchased_products = factory->buyProducts(number_of_products);
    std::list<Product> return_products;
   // std::cout << " Finish buying products " <<  "\n " <<std::endl;



    //  Allocate memory to return value
    int* num_returned = (int*)malloc(sizeof(int)) ;
    (*num_returned) = 0;


   // std::cout << " After allocating memory  "  <<  "\n " <<std::endl;

  //  std::cout << " purchased_products size is   " <<  purchased_products.size()  <<  "\n " <<std::endl;

    // Selects the products that wants to be returned
    std::list<Product>::iterator it_;
    for ( it_ = (purchased_products).begin(); it_ != (purchased_products).end(); it_++ )
        if ( it_->getValue() < min_value ){
            (*num_returned)++;
            (return_products).push_back(*it_);
        }


  //  std::cout << " Number of returning is  " << (*num_returned) <<  "\n " <<std::endl;


    // there's no reason for the company to wait if it doesn't have anything to return.
    if ( (*num_returned) == 0 )
        return num_returned;


    // Performs the function call
    unsigned int id = 0;

   // std::cout << " Before enter returnProducts " <<  "\n " <<std::endl;


    factory->returnProducts((return_products),id );

    return num_returned;


//    // TESTING ARKADI
//    std::list<Product>::iterator it_;
//    for ( it_ = (*purchased_products).begin(); it_ != (*purchased_products).end(); it_++ )
//        std::cout << it_->getId() << std::endl ;




//    std::list<Product> Returned;
//    std::list<Product> Products(company_buyer->products);
    //   std::cout << "Size of products is in wrapper_returnProducts " << Products.size() << "\n " << std::endl;

    //  std::cout << "Size of company_buyer->products is in wrapper_returnProducts " << company_buyer->products.size() << "\n " << std::endl;



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
    if( id == 25 ){
        int multiply_by = 5;
        for (int i = 0; i < num_products*multiply_by ; ++i) {
            Product product(i,i);
            Available.push_back(product);
        }

    }

// ( ARKADI ) for testing only
//    std::list<Product>::iterator it_;
//    for ( it_ = (Available).begin(); it_ != (Available).end(); it_++ )
//        std::cout << it_->getId() << std::endl ;



    // Check arguments
    if( num_products < 0 || min_value < 0 )
        return;

    // We will create a structure that will hold the fields we want to send to the function below
    CompanyBuyer* company_buyer = new CompanyBuyer;
    company_buyer->factory = this;
    company_buyer->num_products=num_products;
    company_buyer->min_value=min_value;


    // Here I create the thread and call the wrapper_buyProducts function and get the answer in the purchased_products_ptr
    pthread_t pthread_buyProducts;
    pthread_create(&pthread_buyProducts, NULL, &wrapper_CompanyBuyer,company_buyer); // create new thread
    std::pair<std::map<unsigned int  , pthread_t>::iterator, bool> res = Threads.insert(std::pair<unsigned int ,pthread_t>(id,pthread_buyProducts)); // insert thread with id to map
   // (ARKADI) DEBUGGING
    if ( ! res.second ) {
        std::cout << "key " <<  id << " already exists "
             << " with value " << (res.first)->second <<  "\n" << std::endl;
    } else {
        std::cout << "created key " << id << " with value " << pthread_buyProducts <<  "\n " <<std::endl;
    }



// ( ARKADI ) TESTING - traversing the list from the end to begin
//    for (std::list<Product>::reverse_iterator it=company_buyer->products.rbegin(); it!=company_buyer->products.rend(); ++it) // traverse list from end to begin
//        std::cout << it->getId() << std::endl ;




}

std::list<Product> Factory::buyProducts(int num_products){


  //  std::cout << "Thread in buyProducts is  " << pthread_self() << "\n " << std::endl;
    // Waiting until it was his turn to buy the products
    pthread_mutex_lock(&global_lock);
 //   std::cout << "Thread took the lock id is " << pthread_self() << "\n " << std::endl;

//    std::cout << "num_of_thieves is " << num_of_thieves << "\n " << std::endl;
//    std::cout << "num_of_waiting_thieves is " << num_of_waiting_thieves << "\n " << std::endl;
//    std::cout << "Available.size()is " << Available.size() << "\n " << std::endl;
//    std::cout << "num_products is " << num_products << "\n " << std::endl;
//    std::cout << "factory_open ==  is " << factory_open << "\n " << std::endl;
//    std::cout << "num_of_companies   is " << num_of_companies << "\n " << std::endl;

    while ( num_of_thieves > 0  || num_of_waiting_thieves > 0 || Available.size() < num_products  ||
            factory_open == false || num_of_companies > 0 )
                pthread_cond_wait(&companies, &global_lock);

  //  std::cout << "Thread is running with id " << pthread_self() << "\n " << std::endl;
    num_of_companies++;
    // Buying num products the oldest products
    std::list<Product> purchased;

    purchased.assign(std::next(Available.begin(), 0), std::next(Available.begin() , num_products));
    Available.erase(std::next(Available.begin(), 0), std::next(Available.begin() , num_products));

    num_of_companies--;
    // Wake the rest of the threads
    pthread_cond_signal(&thieves);
  //  std::cout << "Thread is running keep1 " << pthread_self() << "\n " << std::endl;
    pthread_cond_broadcast(&companies);
  //  std::cout << "Thread is running keep2 " << pthread_self() << "\n " << std::endl;
    pthread_mutex_unlock(&global_lock);
  //  std::cout << "Thread is running keep3 " << pthread_self() << "\n " << std::endl;
    return std::list<Product>(purchased);
}

void Factory::returnProducts(std::list<Product> products,unsigned int id){


//        std::cout << "num_of_thieves is " << num_of_thieves << "\n " << std::endl;
//    std::cout << "num_of_waiting_thieves is " << num_of_waiting_thieves << "\n " << std::endl;
//    std::cout << "Available.size()is " << Available.size() << "\n " << std::endl;
//    std::cout << "factory_open ==  is " << factory_open << "\n " << std::endl;
//    std::cout << "num_of_companies   is " << num_of_companies << "\n " << std::endl;


    // Waiting until it was his turn to buy the products
    pthread_mutex_lock(&global_lock);
    while ( num_of_thieves > 0  || num_of_waiting_thieves > 0 || return_service_open == false ||
            factory_open == false || num_of_companies > 0 )
        pthread_cond_wait(&companies, &global_lock);

    num_of_companies++;
  //  std::cout << "Thread is in returnProducts " << "\n " << std::endl;


    // Returns the incompatible products
    std::list<Product>::iterator it_;
    for ( it_ = products.begin(); it_ != products.end(); it_++ )
        Available.push_back(*it_);



    num_of_companies--;
    // Wake the rest of the threads
    pthread_cond_signal(&thieves);
    pthread_cond_broadcast(&companies);
    pthread_mutex_unlock(&global_lock);

    return ;

}

int Factory::finishCompanyBuyer(unsigned int id){
    if (id < 0 ) return -1;


  //  std::cout << "Thread is in finishCompanyBuyer " <<  "\n " << std::endl;


    // Looking for the thread
    std::map<unsigned int  , pthread_t>::iterator it;
    it = Threads.find(id);
    if (it == Threads.end() )
        std::cout << "id " << id << "is not in map";


 //   std::cout << "Thread id is " << it->second << "\n " << std::endl;


    //std::cout << "Thread waiting join  " << it->second << std::endl;

    // Waiting to recieve the output
    int* res;
    pthread_join(it->second, (void**)&res);


   // std::cout << "Number of products in Available is " << Available.size() <<  " Thread id is "<<   id <<"\n " << std::endl;
   // std::cout << "Number of products returned " << *res <<   " Thread id is "<<  id <<  "\n " << std::endl;

    // remove thread from map
    Threads.erase(it);

    int ret = *res;
    delete(res);    // free allocated integer
    res= nullptr;

    return ret;
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






