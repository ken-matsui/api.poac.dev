mod auth;
mod constants;
mod health;
mod package;
mod routes;
mod schema;
mod user;

use std::env;

use actix_web::{middleware::Logger, web, web::ServiceConfig};
use diesel::r2d2::{ConnectionManager, Pool};
use diesel::PgConnection;
use dotenvy::dotenv;
use shuttle_actix_web::ShuttleActixWeb;

#[shuttle_runtime::main]
async fn actix_web() -> ShuttleActixWeb<impl FnOnce(&mut ServiceConfig) + Send + Clone + 'static> {
    dotenv().ok();

    // Set up database connection pool
    let database_url = env::var("DATABASE_URL").expect("DATABASE_URL must be set");
    let manager = ConnectionManager::<PgConnection>::new(database_url);
    let pool = Pool::builder()
        .build(manager)
        .expect("Failed to create pool.");
    // Set up DB pool to be used with web::Data<Pool> extractor
    let state = web::Data::new(pool.clone());

    let host = env::var("HOST").unwrap_or("127.0.0.1".to_string());
    let port = env::var("PORT")
        .map(|port| port.parse::<u16>().expect("Failed to parse PORT as u16"))
        .unwrap_or(8000);
    log::info!("starting HTTP server at http://{host}:{port}");

    let config = move |cfg: &mut ServiceConfig| {
        cfg.app_data(state)
            .route("/health", web::get().to(health::api))
            .route("/health_db", web::get().to(health::db))
            // .wrap(Logger::default())  TODO: add logger
            .configure(routes::init_routes);
    };
    Ok(config.into())
}
