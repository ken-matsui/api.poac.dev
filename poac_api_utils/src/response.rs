use actix_web::HttpResponse;
use serde::Serialize;

#[derive(Serialize)]
pub struct Response<T> {
    data: T,
}

impl<T: Serialize> Response<T> {
    pub fn new(data: T) -> Self {
        Self { data }
    }

    pub fn ok(data: T) -> HttpResponse {
        HttpResponse::Ok().json(Self::new(data))
    }

    pub fn maybe_ok(data: Option<T>, msg: String) -> HttpResponse {
        if let Some(data) = data {
            Self::ok(data)
        } else {
            HttpResponse::NotFound().json(ErrResponse::new(msg))
        }
    }
}

#[derive(Serialize)]
struct ErrResponse<T> {
    error: T,
}

impl<T: Serialize> ErrResponse<T> {
    fn new(error: T) -> Self {
        Self { error }
    }
}
