mod models;
mod schema;

use actix_web::{middleware::Logger, web, App, HttpServer};
use diesel::r2d2::{ConnectionManager, Pool};
use diesel::PgConnection;
use dotenvy::dotenv;
use std::env;

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    dotenv().ok();
    env_logger::init_from_env(env_logger::Env::new().default_filter_or("info"));

    // set up database connection pool
    let database_url = env::var("DATABASE_URL").expect("DATABASE_URL must be set");
    let manager = ConnectionManager::<PgConnection>::new(database_url);
    let pool = Pool::builder()
        .build(manager)
        .expect("Failed to create pool.");

    log::info!("starting HTTP server at http://localhost:8080");

    // Start HTTP server
    HttpServer::new(move || {
        App::new()
            // set up DB pool to be used with web::Data<Pool> extractor
            .app_data(web::Data::new(pool.clone()))
            .wrap(Logger::default())
        // .service(get_user)
        // .service(add_user)
    })
    .bind(("127.0.0.1", 8080))?
    .run()
    .await
}
