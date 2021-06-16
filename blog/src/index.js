const path = require('path');
const express = require('express');
const handlebars = require('express-handlebars');
const morgan = require('morgan');
const route = require('./routes');
const db = require('./config/db');
const methodOverride = require('method-override');
// require thư mục
const app = express();
const port = 3000;
const sortMiddleware = require('./app/middlewares/sortMiddleware');
db.connect();
// path: localhost:3000 = public
app.use(express.static(path.join(__dirname, 'public')));
app.use(
    express.urlencoded({
        extended: true,
    }),
);
app.use(express.json());
app.use(methodOverride('_method'));

// custom middlewares
app.use(sortMiddleware);
// HTTP logger
//app.use(morgan('combined'))
// Template engine
app.engine(
    'hbs',
    handlebars({
        extname: '.hbs',
        helpers: require('./helpers/handlebars'),
    }),
);
app.set('view engine', 'hbs');
app.set('views', path.join(__dirname, 'resources','views'));

// tich hop vs engine
// req: chứa thông tin yêu cầu gửi lên server
// res: tùy chỉnh kết quả trả về cho client
route(app);

// 127.0.0.1 = localhost
app.listen(port, () => {
    console.log(`App listening at http://localhost:${port}`);
});
