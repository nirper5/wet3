//
// Created by Arkadi Gurevich on 26/05/2018.
//

#include <unistd.h>
#include "Factory.h"

typedef struct produce_t{
    int num_products;
    Product* products;
    Factory* factory;
} Produce;


typedef struct company_buyer_t{
    int num_products;
    int min_value;
    Factory* factory;
} CompanyBuyer;


typedef struct thief_t{
    int num_products;
    unsigned int fake_id;
    Factory* factory;
} Thief;


static void* wrapper_produce(void* new_produce) {

    // std::cout << "insert to wrapper produce" << std::endl;

    auto *new_produce_c = static_cast<Produce *> (new_produce);
    Factory *factory = new_produce_c->factory;
    int num_products = new_produce_c->num_products;
    Product *products = new_produce_c->products;


    delete new_produce_c;

    // std::cout << "call produce" << std::endl;

    factory->produce(num_products, products);
    return nullptr;
}

static void* wrapper_simple_buyer(void* factory){

    //   std::cout<< "insert to wrapper_simple_buyer" <<std::endl;

    auto new_factory = static_cast<Factory *> (factory);
    int* res = new int;
    *res = new_factory->tryBuyOne();
    return res;
}


static void* wrapper_startThief(void* sp){


    // Extract the parameters of the thief
    Thief* struct_sp  = static_cast<Thief*>(sp);
    unsigned int fake_id = struct_sp->fake_id;
    int num_products = struct_sp->num_products;
    Factory* factory = struct_sp->factory;


    // free memory of struct because we will not use it again
    delete struct_sp;
    struct_sp = nullptr;

    // Allocate a place to the parameter back from the thief's function
    int* stolen_products = new int;
    if ( stolen_products == nullptr)
        return NULL;

    // Initialization
    (*stolen_products) = 0;

    (*stolen_products) =  factory->stealProducts(num_products,fake_id);


    return (stolen_products);

}

static void* wrapper_CompanyBuyer(void* company){


    // casting to company_buyer
    CompanyBuyer* company_buyer = static_cast<CompanyBuyer*>(company);

    // Extracts the fields from the struct
    int number_of_products = company_buyer->num_products;
    Factory* factory = company_buyer->factory;
    int min_value = company_buyer->min_value;

    // free memory of struct because we will not use it again
    delete company_buyer;
    company_buyer = nullptr;

    // A list containing the purchased products
    std::list<Product> purchased_products = factory->buyProducts(number_of_products);

    //  Allocate memory to return value
    int* num_returned = new int ;
    if ( num_returned == nullptr )
        return NULL;

    // Initialization
    (*num_returned) = 0;

    // Selects the products that wants to be returned
    std::list<Product>::iterator it_;
    std::list<Product> return_products;
    for ( it_ = (purchased_products).begin(); it_ != (purchased_products).end(); it_++ )
        if ( it_->getValue() < min_value ){
            (*num_returned)++;
            (return_products).push_back(*it_);
        }


    // there's no reason for the company to wait if it doesn't have anything to return.
    if ( (*num_returned) == 0 )
        return num_returned;


    // Performs the function call
    unsigned int id = 0;
    factory->returnProducts((return_products),id );

    return num_returned;


//    // TESTING ARKADI
//    std::list<Product>::iterator it_;
//    for ( it_ = (*purchased_products).begin(); it_ != (*purchased_products).end(); it_++ )
//        std::cout << it_->getId() << std::endl ;


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

    //nir add this
    pthread_mutexattr_init(&Attr);
    pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_ERRORCHECK);

    pthread_mutex_init(&global_lock, &Attr);


}

Factory::~Factory(){

    for (std::map<unsigned int, pthread_t>::iterator it = Threads.begin(); it != Threads.end(); ++it)
        pthread_cancel(it->second);

}

void Factory::startProduction(int num_products, Product* products,unsigned int id){

    //   std::cout<<"enter to startProduction" <<std::endl;

    if(num_products <=0 || products == nullptr ){
        return;
    }

    Produce* new_produce = new Produce;
    new_produce->factory=this;
    new_produce->num_products=num_products;
    new_produce->products=products;

    pthread_t pthread_produce;
    pthread_create(&pthread_produce, NULL, &wrapper_produce ,new_produce);

    std::pair<std::map<unsigned int  , pthread_t>::iterator, bool> res = Threads.insert(std::pair<unsigned int ,pthread_t>(id,pthread_produce)); // insert thread with id to map

    // (Nir) DEBUGGING
//    if ( ! res.second ) {
//        std::cout << "key " <<  id << " already exists "
//                  << " with value " << (res.first)->second <<  "\n" << std::endl;
//    } else {
//        std::cout << "created key " << id << " with value " << pthread_produce <<  "\n " <<std::endl;
//    }
}

void Factory::produce(int num_products, Product* products){

    //  std::cout<<"insert to produce"<<std::endl;

    if(num_products <= 0 || products== nullptr) return;

    pthread_mutex_lock(&global_lock);

    for (int i = 0 ; i < num_products ; i++) {

        //   std::cout << "insert product num:" << products[i].getValue() << std::endl;

        Available.push_back(products[i]);
    }

    //   std::cout<<"current in available:"<<std::endl;
//    for(auto v : Available){
//        std::cout << v.getId() << std::endl;
//    }

    // Wake the rest of the threads
    pthread_cond_signal(&thieves);
    //     std::cout << "Thread is running keep1 " << pthread_self() << "\n " << std::endl;
    pthread_cond_broadcast(&companies);

    pthread_mutex_unlock(&global_lock);

    //   std::cout << "return from produce"<<std::endl;
}

void Factory::finishProduction(unsigned int id){

    //  std::cout << "Thread is in finishProduction " <<  "\n " << std::endl;


    // Looking for the thread
    std::map<unsigned int  , pthread_t>::iterator it;
    it = Threads.find(id);
    if (it == Threads.end() )
        std::cout << "id " << id << "is not in map";


    // std::cout << "Thread id is " << it->second << "\n " << std::endl;

    int* res;
    pthread_join(it->second, (void**)&res);

    //std::cout << "Thread waiting join  " << it->second << std::endl;

    //no need for return value
    // remove thread from map
    Threads.erase(it);
    return;
}

void Factory::startSimpleBuyer(unsigned int id){

    //std::cout << "insert to startSimpleBuyer" << std::endl;

    pthread_t pthread_simple_buyer;
    pthread_create(&pthread_simple_buyer, NULL, &wrapper_simple_buyer, this);
    std::pair<std::map<unsigned int  , pthread_t>::iterator, bool> res = Threads.insert(std::pair<unsigned int ,pthread_t>(id,pthread_simple_buyer)); // insert thread with id to map

//    if ( ! res.second ) {
//        std::cout << "key " <<  id << " already exists "
//                  << " with value " << (res.first)->second <<  "\n" << std::endl;
//    } else {
//        std::cout << "created key " << id << " with value " << pthread_simple_buyer <<  "\n " <<std::endl;
//    }

}

int Factory::tryBuyOne(){

    //simple buyer can get the lock only if there is other thread working.
    int res = pthread_mutex_trylock(&global_lock);
    if(res != 0){

        // std::cout<<"cannot open the mutex in try buy one with res:"<< res <<std::endl;

        return -1;
    }

    //right here the lock is occupied
    if(!factory_open || Available.size() <= 0 ){

        //   std::cout<<"in try buy one availble size is:" << Available.size() << std::endl;

        pthread_mutex_unlock(&global_lock);
        return -1;
    }
    num_of_buyers++;

    // std::cout<<"pop one product in tryBuyOne"<<std::endl;

    std::list<Product>::iterator it = Available.begin();
    int product_id = (*it).getId();

    //  std::cout<<"product_id_erase="<<product_id<<std::endl;

    Available.erase(it);
    num_of_buyers--;

    // Wake the rest of the threads
    pthread_cond_signal(&thieves);
    //     std::cout << "Thread is running keep1 " << pthread_self() << "\n " << std::endl;
    pthread_cond_broadcast(&companies);

    pthread_mutex_unlock(&global_lock);
    return product_id;
}

int Factory::finishSimpleBuyer(unsigned int id){

//    std::cout << "insert to finishSimpleBuyer" << std::endl;

    std::map<unsigned int  , pthread_t>::iterator it;
    it = Threads.find(id);
    if (it == Threads.end() ){
        std::cout << "id " << id << "is not in map" << std::endl;
//        std::cout << "current threads in the map:" << std::endl;
//        for(auto v : Threads){
//            std::cout << v.first << std::endl;
//        }
    }


    int* res;

    //   std::cout<< "waiting for thread finis in finishSimpleBuyer" << std::endl;

    pthread_join(it->second, (void**)&res);


    //    std::cout << "after join in try buy one Number of products in Available is " << Available.size() <<  " Thread id is "<<   id <<"\n " << std::endl;
    // std::cout << "Number of products returned " << *res <<   " Thread id is "<<  id <<  "\n " << std::endl;

    unsigned int to_remove= (*it).first;
    //  std::cout<<"before erase thread num:"<< to_remove <<std::endl;
    // remove thread from map
    Threads.erase(it);

    //  std::cout<<"after erase thread num:"<< to_remove <<std::endl;

    int ret = *res;
    delete(res);    // free allocated integer
    res= nullptr;

    return ret;
}


int j=0;
void Factory::startCompanyBuyer(int num_products, int min_value,unsigned int id){


//    // (ARKADI ) for TESTING  Insert num_products elements to the end of the list
        int added_items = 9;
        for (int i = 0; i < added_items ; ++i) {
            Product product(j,j);
            j++;
            Available.push_back(product);
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
    if (company_buyer == nullptr)
        return;
    company_buyer->factory = this;
    company_buyer->num_products=num_products;
    company_buyer->min_value=min_value;


    // Here I create the thread and call the wrapper_buyProducts function and get the answer in the purchased_products_ptr
    pthread_t pthread_startCompanyBuyer;
    pthread_create(&pthread_startCompanyBuyer, NULL, &wrapper_CompanyBuyer,company_buyer); // create new thread
    std::pair<std::map<unsigned int  , pthread_t>::iterator, bool> res = Threads.insert(std::pair<unsigned int ,pthread_t>(id,pthread_startCompanyBuyer)); // insert thread with id to map


}

std::list<Product> Factory::buyProducts(int num_products){


    // Waiting until it was his turn to buy the products
    pthread_mutex_lock(&global_lock);

    // The company waits until all conditions are met
    while ( num_of_thieves > 0  || num_of_waiting_thieves > 0 || Available.size() < num_products  ||
            factory_open == false || num_of_companies > 0 || num_of_buyers > 0  )
                pthread_cond_wait(&companies, &global_lock);

    num_of_companies++;
    // Buying num products the oldest products
    std::list<Product> purchased;
    purchased.assign(std::next(Available.begin(), 0), std::next(Available.begin() , num_products));
    Available.erase(std::next(Available.begin(), 0), std::next(Available.begin() , num_products));

    num_of_companies--;
    // Wake the rest of the threads
    pthread_cond_signal(&thieves);
    pthread_cond_broadcast(&companies);
    pthread_mutex_unlock(&global_lock);
    return std::list<Product>(purchased);
}

void Factory::returnProducts(std::list<Product> products,unsigned int id){


    // The company waits until all conditions are met
    pthread_mutex_lock(&global_lock);
    while ( num_of_thieves > 0  || num_of_waiting_thieves > 0 || return_service_open == false ||
            factory_open == false || num_of_companies > 0 || num_of_buyers > 0  )
        pthread_cond_wait(&companies, &global_lock);

    num_of_companies++;

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

    // Looking for the thread
    std::map<unsigned int  , pthread_t>::iterator it;
    it = Threads.find(id);
    if (it == Threads.end() )
        std::cout << "id " << id << "is not in map";


    // Waiting to receive the output
    int* res;
    pthread_join(it->second, (void**)&res);


  //  std::cout << "Company id finished is " << id << " and return " << *res << " products. Now the number of Available products is " << Available.size() << std::endl;

    // remove thread from map
    Threads.erase(it);

    int ret = *res;
    delete(res);    // free allocated integer
    res= nullptr;

    return ret;
}

void Factory::startThief(int num_products,unsigned int fake_id){

    if (num_products < 0)
        return;

    //     The struct sent to the wrapper function
    Thief* thief_ = new Thief;
    if ( thief_ == nullptr)
        return;
    thief_->factory = this;
    thief_->num_products=num_products;
    thief_->fake_id=fake_id;

    // Now another thief has entered the waiting list
    pthread_mutex_lock(&global_lock);
    num_of_waiting_thieves++;
    pthread_mutex_unlock(&global_lock);

    // We will create a thread with a call to the wrapper function
    pthread_t pthread_startThief;
    pthread_create(&pthread_startThief, NULL, &wrapper_startThief,thief_); // create new thread
    std::pair<std::map<unsigned int  , pthread_t>::iterator, bool> res = Threads.insert(std::pair<unsigned int ,pthread_t>(fake_id,pthread_startThief)); // insert thread with id to map

    return;

}

int Factory::stealProducts(int num_products,unsigned int fake_id){

    // Testing parameters
    if ( num_products < 0 ) return -1;


//    std::cout << "Factory is open ?? " << this->factory_open  << std::endl;


    pthread_mutex_lock(&global_lock);

    // Waiting for the thief's turn to steal
    while ( num_of_thieves > 0  || num_of_buyers > 0 ||  num_of_companies>0 || factory_open == false  )
        pthread_cond_wait(&thieves, &global_lock);



    num_of_thieves++;
    num_of_waiting_thieves--;

    // Selects how many products to steal depending on how many products there are in the factory
    // and how many products you would like to steal
    int num_stolen = ( num_products < (int)Available.size() ) ? num_products : (int)Available.size();



    // Stealing products
    std::list<Product>::iterator it_;
    int stolen_cnt = 0 ;

    for ( it_ = (Available).begin(); it_ != (Available).end(); it_++ )
    {
        if (stolen_cnt >= num_stolen) break;
            stolen_cnt++;
        Stolen.push_back(std::pair<Product,int>(*it_,fake_id));
    }
    Available.erase(std::next(Available.begin(), 0), std::next(Available.begin() , num_stolen)); // Deletes the products the thief stole from the factory


    num_of_thieves--;
    // Wake the rest of the threads
    pthread_cond_signal(&thieves);
    pthread_cond_broadcast(&companies);
    pthread_mutex_unlock(&global_lock);

    return num_stolen;
}



int Factory::finishThief(unsigned int fake_id){

    if (fake_id < 0 ) return -1;

    // Looking for the thread
    std::map<unsigned int  , pthread_t>::iterator it;
    it = Threads.find(fake_id);
    if (it == Threads.end() )
        std::cout << "id " << fake_id << "is not in map";
    

    // Waiting to receive the output
    int* res;
    pthread_join(it->second, (void**)&res);


    std::cout << "Thief id finished is " << fake_id << " and stole " << *res << " products. Now the number of Available products is " << Available.size() <<
            " Number of Stolen Items is " << Stolen.size() <<  " " << std::endl;

    // remove thread from map
    Threads.erase(it);

    int ret = *res;
    delete(res);    // free allocated integer
    res= nullptr;

    return ret;

}

void Factory::closeFactory(){
    factory_open = false;
}

void Factory::openFactory(){
    factory_open = true;
    pthread_cond_signal(&thieves);
    pthread_cond_broadcast(&companies);
}

void Factory::closeReturningService(){
    return_service_open = false;
}

void Factory::openReturningService(){
    return_service_open = true;
    pthread_cond_broadcast(&companies);
}

std::list<std::pair<Product, int>> Factory::listStolenProducts(){

    pthread_mutex_lock(&global_lock);
    std::list<std::pair<Product, int>> list_stolen(Stolen);
    pthread_mutex_unlock(&global_lock);
    return std::list<std::pair<Product, int>>(list_stolen);
}

std::list<Product> Factory::listAvailableProducts(){

    pthread_mutex_lock(&global_lock);
    std::list<Product> list_available(Available);
    pthread_mutex_unlock(&global_lock);


    return std::list<Product>(list_available);
}






